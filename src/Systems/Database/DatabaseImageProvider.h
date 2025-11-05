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
struct SResourceBundle;
typedef std::shared_ptr<SProject> tspProject;
typedef std::shared_ptr<SResource> spResource;
typedef std::shared_ptr<SResourceBundle>tspResourceBundle;

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
                      const QString& sResourceName,
                      const QString& sResourceBundleName,
                      const QUrl& sResourcePath,
                      QSize* pSize, const QSize& requestedSize,
                      bool bLoadedBefore);
  QImage LoadImage(const QString& sPath);
  QImage RequestMovieFrame(tspProject spProject,
                           spResource spResource,
                           const QString& sResourceName,
                           const QString& sResourceBundleName,
                           const QUrl& sResourcePath,
                           QSize* pSize, const QSize& requestedSize,
                           bool bLoadedBefore);

  std::weak_ptr<CDatabaseManager>            m_wpDatabase;
};

#endif // CDATABASEIMAGEPROVIDER_H
