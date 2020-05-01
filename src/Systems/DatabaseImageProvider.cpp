#include "DatabaseImageProvider.h"
#include "Systems/DatabaseManager.h"
#include "Systems/Project.h"
#include "Systems/Resource.h"
#include <QFileInfo>
#include <QImageReader>

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
      auto spResource = spDbManager->FindResource(spProject, vsParts[1]);
      if (nullptr != spResource)
      {
        QReadLocker locker(&spResource->m_rwLock);
        if (spResource->m_type._to_integral() == EResourceType::eImage)
        {
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
