#ifndef CDATABASEIMAGEPROVIDER_H
#define CDATABASEIMAGEPROVIDER_H

#include <QObject>
#include <QQuickImageProvider>
#include <QReadLocker>
#include <memory>

class CDatabaseManager;
class QNetworkAccessManager;
struct SProject;
struct SResource;
typedef std::shared_ptr<SProject> tspProject;
typedef std::shared_ptr<SResource> spResource;

class CDatabaseImageProvider : public QObject, public QQuickImageProvider
{
  Q_OBJECT

public:
  explicit CDatabaseImageProvider(const std::weak_ptr<CDatabaseManager>& wpDatabase);
  ~CDatabaseImageProvider();

  QImage requestImage(const QString& id, QSize* pSize, const QSize& requestedSize) override;

private:
  QImage RequestImage(tspProject spProject,
                      spResource spResource,
                      std::shared_ptr<CDatabaseManager> spDbManager,
                      QSize* pSize, const QSize& requestedSize,
                      bool bLoadedBefore,
                      QReadLocker& locker);
  QImage LoadImage(const QString& sPath);
  QImage RequestMovieFrame(tspProject spProject,
                           spResource spResource,
                           QSize* pSize, const QSize& requestedSize,
                           bool bLoadedBefore,
                           QReadLocker& locker);

  std::weak_ptr<CDatabaseManager>            m_wpDatabase;
};

#endif // CDATABASEIMAGEPROVIDER_H
