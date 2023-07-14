#ifndef PROJECT_H
#define PROJECT_H

#include "ISerializable.h"
#include "DatabaseInterface/ProjectData.h"
#include "Enums.h"
#include "Resource.h"
#include "ResourceBundle.h"
#include "Scene.h"
#include "Tag.h"
#include <QObject>
#include <QPointer>
#include <QReadWriteLock>
#include <QSharedPointer>
#include <map>
#include <memory>

class QJSEngine;

struct SProject : public ISerializable, std::enable_shared_from_this<SProject>,
                  public SProjectData
{
  SProject();
  SProject(const SProject& other);
  ~SProject() override;

  mutable QReadWriteLock    m_rwLock;
  QStringList               m_vsKinks;
  tvspScene                 m_vspScenes;
  tspResourceMap            m_spResourcesMap;
  tspResourceBundleMap      m_spResourceBundleMap;
  tspTagMap                 m_vspTags;
  QStringList               m_vsMountPoints;          // Archives need to be mounted via physFS to access content
                                                      // WARNING: root is NOT included in this list

  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;

  std::shared_ptr<SProject> GetPtr()
  {
    return shared_from_this();
  }
};

//----------------------------------------------------------------------------------------
//
typedef std::shared_ptr<SProject> tspProject;
typedef std::vector<tspProject>   tvspProject;

Q_DECLARE_METATYPE(tspProject)

//----------------------------------------------------------------------------------------
//
QString PhysicalProjectName(const tspProject& spProject);

//----------------------------------------------------------------------------------------
//
QString PhysicalProjectPath(const tspProject& spProject);

//----------------------------------------------------------------------------------------
//
bool ProjectNameCheck(const QString& sProjectName, QString* sErrorText = nullptr);

//----------------------------------------------------------------------------------------
//
QString ToValidProjectName(const QString& sProjectName);

#endif // PROJECT_H
