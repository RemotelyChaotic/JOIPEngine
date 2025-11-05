#include "DatabaseImageProvider.h"
#include "Systems/DatabaseManager.h"

#include "Systems/Database/Project.h"
#include "Systems/Database/Resource.h"
#include "Systems/Database/ResourceBundle.h"

#include <QtAV/VideoFrameExtractor.h>
#include <QFileInfo>
#include <QImageReader>
#include <QNetworkAccessManager>
#include <QNetworkReply>

CDatabaseImageProvider::CDatabaseImageProvider(const std::weak_ptr<CDatabaseManager>& wpDatabase) :
  QObject(nullptr),
  QQuickImageProvider(QQuickImageProvider::Image, QQmlImageProviderBase::ForceAsynchronousImageLoading),
  m_wpDatabase(wpDatabase)
{
}

CDatabaseImageProvider::~CDatabaseImageProvider()
{

}

//----------------------------------------------------------------------------------------
//
QImage CDatabaseImageProvider::requestImage(const QString& id, QSize* pSize,
                                            const QSize& requestedSize)
{
  QStringList vsParts = id.split("/");
  if (2 == vsParts.length())
  {
    if (auto spDbManager = m_wpDatabase.lock())
    {
      auto spProject = spDbManager->FindProject(vsParts[0].toInt());
      if (nullptr == spProject)
      {
        return QImage();
      }

      QReadLocker projLocker(&spProject->m_rwLock);
      bool bLoadedBefore = spProject->m_bLoaded;
      projLocker.unlock();

      auto spResource = spDbManager->FindResourceInProject(spProject, vsParts[1]);
      if (nullptr != spResource)
      {
        QReadLocker locker(&spResource->m_rwLock);
        const QString sResourceName = spResource->m_sName;
        const QUrl sResourcePath = spResource->m_sPath;
        const QString sResourceBundle = spResource->m_sResourceBundle;
        if (spResource->m_type._to_integral() == EResourceType::eImage)
        {
          locker.unlock();
          return RequestImage(spProject, spResource, spDbManager,
                              sResourceName, sResourceBundle, sResourcePath,
                              pSize, requestedSize, bLoadedBefore);
        }
        else if (spResource->m_type._to_integral() == EResourceType::eMovie)
        {
          locker.unlock();
          return RequestMovieFrame(spProject, spResource,
                                   sResourceName, sResourceBundle, sResourcePath,
                                   pSize, requestedSize, bLoadedBefore);
        }
      }
    }
  }
  return QImage();
}

//----------------------------------------------------------------------------------------
//
QImage CDatabaseImageProvider::RequestImage(tspProject spProject,
                                            spResource spResource,
                                            std::shared_ptr<CDatabaseManager> spDbManager,
                                            const QString& sResourceName,
                                            const QString& sResourceBundleName,
                                            const QUrl& sResourcePath,
                                            QSize* pSize, const QSize& requestedSize,
                                            bool bLoadedBefore)
{
  // local file
  if (IsLocalFile(sResourcePath))
  {
    QString sPath = ResourceUrlToAbsolutePath(spResource);

    CDatabaseManager::LoadProject(spProject);
    CDatabaseManager::LoadBundle(spProject, sResourceBundleName);
    if (QFileInfo(sPath).exists())
    {
      QImage img = LoadImage(sPath);
      if (!img.isNull())
      {
        if (nullptr != pSize)
        {
          *pSize = img.size();
        }

        if (!bLoadedBefore)
        {
          // unload resources again to save memory
          if (!CDatabaseManager::UnloadProject(spProject))
          {
            qWarning() << tr("Unload of resources failed:") << sResourceName;
          }
        }
        return img.scaled(0 < requestedSize.width() ? requestedSize.width() : img.width(),
                          0 < requestedSize.height() ? requestedSize.height() : img.height(),
                          Qt::KeepAspectRatio, Qt::SmoothTransformation);
      }
    }

    if (!bLoadedBefore) { CDatabaseManager::UnloadProject(spProject); }
  }
  // remote resource
  else
  {
    QImage img;
    QEventLoop loop;
    std::shared_ptr<QNetworkAccessManager> spManager = std::make_shared<QNetworkAccessManager>();
    QPointer<QNetworkReply> pReply = spManager->get(QNetworkRequest(sResourcePath));
    connect(pReply, &QNetworkReply::finished,
            this, [pReply, &spDbManager, &img, &loop](){
      if(nullptr != pReply)
      {
        QUrl url = pReply->url();
        QByteArray arr = pReply->readAll();

        qint32 iLastIndex = url.fileName().lastIndexOf('.');
        const QString sFileName = url.fileName();
        QString sFormat = "*" + sFileName.mid(iLastIndex, sFileName.size() - iLastIndex);

        QStringList imageFormatsList = SResourceFormats::ImageFormats();
        if (nullptr != spDbManager)
        {
          if (imageFormatsList.contains(sFormat))
          {
            QPixmap mPixmap;
            mPixmap.loadFromData(arr);
            if (!mPixmap.isNull())
            {
              img = mPixmap.toImage();
            }
          }
        }
        bool bOk = QMetaObject::invokeMethod(&loop, "quit", Qt::QueuedConnection);
        assert(bOk); Q_UNUSED(bOk)
      }
      else
      {
        qCritical() << "QNetworkReply object was destroyed too early.";
      }
    });
    connect(pReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this, [pReply, &loop](QNetworkReply::NetworkError error){
      Q_UNUSED(error)
      if (nullptr != pReply)
      {
        qWarning() << tr(QT_TR_NOOP("Error fetching remote resource: %1"))
                      .arg(pReply->errorString());
        bool bOk = QMetaObject::invokeMethod(&loop, "quit", Qt::QueuedConnection);
        assert(bOk); Q_UNUSED(bOk)
      }
      else
      {
        qCritical() << "QNetworkReply object was destroyed too early.";
      }
    });

    loop.exec();

    if (nullptr != pReply) { delete pReply; }

    if (nullptr != pSize)
    {
      *pSize = img.size();
    }

    return img.scaled(0 < requestedSize.width() ? requestedSize.width() : img.width(),
                      0 < requestedSize.height() ? requestedSize.height() : img.height(),
                      Qt::KeepAspectRatio, Qt::SmoothTransformation);
  }

  return QImage();
}

//----------------------------------------------------------------------------------------
//
QImage CDatabaseImageProvider::LoadImage(const QString& sPath)
{
  QImageReader reader(sPath);
  if (reader.canRead())
  {
    return reader.read();
  }
  return QImage();
}

//----------------------------------------------------------------------------------------
//
QImage CDatabaseImageProvider::RequestMovieFrame(tspProject spProject,
                                                 spResource spResource,
                                                 const QString& sResourceName,
                                                 const QString& sResourceBundleName,
                                                 const QUrl& sResourcePath,
                                                 QSize* pSize, const QSize& requestedSize,
                                                 bool bLoadedBefore)
{
  std::shared_ptr<QtAV::VideoFrameExtractor> spExtractor =
      std::make_shared<QtAV::VideoFrameExtractor>();
  spExtractor->setAsync(false);

  // local file
  bool bOk = true;
  if (IsLocalFile(sResourcePath))
  {
    QString sPath = ResourceUrlToAbsolutePath(spResource);

    CDatabaseManager::LoadProject(spProject);
    CDatabaseManager::LoadBundle(spProject, sResourceBundleName);
    if (QFileInfo(sPath).exists())
    {
      spExtractor->setSource(sPath);
    }
    else
    {
      bOk = false;
    }
  }
  else
  {
    CDatabaseManager::LoadProject(spProject);
    spExtractor->setSource(sResourcePath.toString());
  }

  QImage img;
  if (bOk)
  {
    QMetaObject::Connection connErr =
        connect(spExtractor.get(), &QtAV::VideoFrameExtractor::error,
                this, [&bOk](const QString& errorMessage) {
      qWarning() << tr("Image extraction failed:") << errorMessage;
      bOk = false;
    });
    QMetaObject::Connection connFrame =
        connect(spExtractor.get(), &QtAV::VideoFrameExtractor::frameExtracted,
                this, [&bOk, &img, pSize](const QtAV::VideoFrame& frame) {
      bOk = true;
      img = frame.toImage();
      if (nullptr != pSize)
      {
        *pSize = img.size();
      }
    });

    // run
    spExtractor->setPosition(1);
    spExtractor->extract();

    // reset
    disconnect(connErr);
    disconnect(connFrame);
    spExtractor->setSource(QString());
  }

  if (!bLoadedBefore)
  {
    // unload resources again to save memory
    if (!CDatabaseManager::UnloadProject(spProject))
    {
      qWarning() << tr("Unload of resources failed:") << sResourceName;
    }
  }

  return img.scaled(0 < requestedSize.width() ? requestedSize.width() : img.width(),
                    0 < requestedSize.height() ? requestedSize.height() : img.height(),
                    Qt::KeepAspectRatio, Qt::SmoothTransformation);
}
