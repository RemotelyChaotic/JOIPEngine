#ifndef CDATABASEIMAGEPROVIDER_H
#define CDATABASEIMAGEPROVIDER_H

#include "Systems/DatabaseInterface/ResourceData.h"

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
typedef std::shared_ptr<SResource> tspResource;
typedef std::shared_ptr<SResourceBundle>tspResourceBundle;

class CProjectProvider
{
public:
  CProjectProvider();
  virtual ~CProjectProvider();

  virtual tspProject FindProject(qint32 iId);
  virtual tspResource FindResourceInProject(const tspProject& spProject, const QString& sName);

protected:
  std::weak_ptr<CDatabaseManager>            m_wpDatabase;
};

//----------------------------------------------------------------------------------------
//
class CDatabaseImageProvider : public QObject, public QQuickImageProvider
{
  Q_OBJECT

public:
  explicit CDatabaseImageProvider(const std::shared_ptr<CProjectProvider>& spProjectProvider);
  ~CDatabaseImageProvider();

  QImage requestImage(const QString& id, QSize* pSize, const QSize& requestedSize) override;

private:
  QImage RequestImage(tspProject spProject,
                      tspResource spResource,
                      const QString& sResourceName,
                      const QString& sResourceBundleName,
                      const SResourcePath& sResourcePath,
                      QSize* pSize, const QSize& requestedSize,
                      bool bLoadedBefore);
  QImage LoadImage(const QString& sPath);
  QImage RequestMovieFrame(tspProject spProject,
                           tspResource spResource,
                           const QString& sResourceName,
                           const QString& sResourceBundleName,
                           const SResourcePath& sResourcePath,
                           QSize* pSize, const QSize& requestedSize,
                           bool bLoadedBefore);

  std::shared_ptr<CProjectProvider>          m_spProjectProvider;
};

#endif // CDATABASEIMAGEPROVIDER_H
