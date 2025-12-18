#ifndef PROJECT_H
#define PROJECT_H

#include "ISerializable.h"
#include "Systems/DatabaseInterface/ProjectData.h"
#include "Enums.h"
#include "Resource.h"
#include "ResourceBundle.h"
#include "SaveData.h"
#include "Scene.h"
#include "Tag.h"
#include <QObject>
#include <QPointer>
#include <QReadWriteLock>
#include <QSharedPointer>
#include <map>
#include <memory>

class QJSEngine;

struct SRuntimeData
{
  QStringList               m_vsKinks;
  tvspScene                 m_vspScenes;
  tspResourceMap            m_spResourcesMap;
  tspResourceBundleMap      m_spResourceBundleMap;
  tspTagMap                 m_vspTags;
  tspSaveDataMap            m_vspAchievements;

  void Clear();
  void MergeIntoThis(const SRuntimeData& data, std::shared_ptr<SProject> spProject);
};

//----------------------------------------------------------------------------------------
//
struct SProject : public ISerializable, std::enable_shared_from_this<SProject>,
                  public SProjectData
{
  SProject();
  SProject(const SProject& other);
  ~SProject() override;

  mutable QReadWriteLock    m_rwLock;
  SRuntimeData              m_baseData;
  QStringList               m_vsMountPoints;          // Archives need to be mounted via physFS to access content
                                                      // WARNING: root is NOT included in this list

  std::vector<std::shared_ptr<SProject>>
                            m_vspPlugins; // only populated if the project is loaded
  SRuntimeData              m_pluginData; // only populated if the project is loaded

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
bool ProjectNameCheck(const QString& sProjectName, QString* sErrorText = nullptr,
                      const tspProject& spExclude = nullptr);

//----------------------------------------------------------------------------------------
//
QString ToValidProjectName(const QString& sProjectName);

#endif // PROJECT_H
