#include "DatabaseImageProvider.h"
#include "Systems/DatabaseManager.h"
#include "Systems/Project.h"
#include "Systems/Resource.h"
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
      auto spResource = spDbManager->FindResourceInProject(spProject, vsParts[1]);
      if (nullptr != spResource)
      {
        QReadLocker locker(&spResource->m_rwLock);
        if (spResource->m_type._to_integral() == EResourceType::eImage)
        {
          // local file
          if (spResource->m_sPath.isLocalFile())
          {
            QString sPath = ResourceUrlToAbsolutePath(spResource->m_sPath,
                                                      PhysicalProjectName(spProject));
            if (QFileInfo(sPath).exists())
            {
              QImage img = LoadImage(sPath);
              if (!img.isNull())
              {
                if (nullptr != pSize)
                {
                  *pSize = img.size();
                }
                return img.scaled(0 < requestedSize.width() ? requestedSize.width() : img.width(),
                                  0 < requestedSize.height() ? requestedSize.height() : img.height(),
                                  Qt::KeepAspectRatio, Qt::SmoothTransformation);
              }
            }
          }
          // remote resource
          else
          {
            QImage img;
            QEventLoop loop;
            std::shared_ptr<QNetworkAccessManager> spManager = std::make_shared<QNetworkAccessManager>();
            QPointer<QNetworkReply> pReply = spManager->get(QNetworkRequest(spResource->m_sPath));
            connect(pReply, &QNetworkReply::finished,
                    this, [pReply, &spDbManager, &img, &loop](){
              if(nullptr != pReply)
              {
                QUrl url = pReply->url();
                QByteArray arr = pReply->readAll();

                qint32 iLastIndex = url.fileName().lastIndexOf('.');
                const QString sFileName = url.fileName();
                QString sFormat = "*" + sFileName.mid(iLastIndex, sFileName.size() - iLastIndex);

                QStringList imageFormatsList = ImageFormats();
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

            return img.scaled(0 < requestedSize.width() ? requestedSize.width() : img.width(),
                              0 < requestedSize.height() ? requestedSize.height() : img.height(),
                              Qt::KeepAspectRatio, Qt::SmoothTransformation);
          }
        }
      }
    }
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
    if (!reader.supportsAnimation())
    {
      return reader.read();
    }
    else
    {
      emit SignalTriedToLoadMovie(sPath);
    }
  }
  return QImage();
}
