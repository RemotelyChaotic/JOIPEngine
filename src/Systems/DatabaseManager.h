#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include "ThreadedSystem.h"
#include "Database/Kink.h"
#include "Database/Resource.h"
#include <QAtomicInt>
#include <QDir>
#include <QMutex>
#include <set>

class CDatabaseData;
class CDatabaseIO;
class CProjectScriptWrapper;
class CSceneScriptWrapper;
class CSettings;
struct SProject;
struct SSaveData;
struct SScene;
struct SResourceBundle;
struct STag;
typedef std::shared_ptr<SProject>       tspProject;
typedef std::shared_ptr<SResourceBundle>tspResourceBundle;
typedef std::shared_ptr<SSaveData>      tspSaveData;
typedef std::shared_ptr<SScene>         tspScene;
typedef std::shared_ptr<STag>           tspTag;

typedef std::vector<std::function<void(const tspProject&)>> tvfnActionsProject;
typedef std::vector<std::function<void(const tspScene&)>> tvfnActionsScene;
typedef std::vector<std::function<void(const tspResource&)>> tvfnActionsResource;

class CDatabaseManager : public CSystemBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CDatabaseManager)

public:
  CDatabaseManager();
  ~CDatabaseManager() override;

  static bool LoadBundle(tspProject& spProject, const QString& sBundle);
  static bool LoadPlugins(tspProject& spProject);
  static bool LoadProject(tspProject& spProject, bool bLoadPlugins);
  static bool SetProjectEditing(tspProject& spProject, bool bEnabled);
  static bool UnloadBundle(tspProject& spProject, const QString& sBundle);
  static bool UnloadProject(tspProject& spProject);
  static bool UnloadPlugins(tspProject& spProject);

  // Project
  qint32 AddProject(const QString& sDirName = "New_Project", quint32 iVersion = 1,
                    bool bBundled = false, bool bReadOnly = false,
                    const tvfnActionsProject& vfnActionsAfterAdding = tvfnActionsProject());
  void ClearProjects();
  bool DeserializeProject(qint32 iId);
  bool DeserializeProject(const QString& sName);
  tspProject FindProject(qint32 iId);
  tspProject FindProject(const QString& sName);
  std::set<qint32, std::less<qint32>> ProjectIds();
  bool PrepareNewProject(qint32 iId);
  bool PrepareNewProject(const QString& sName);
  void RemoveProject(qint32 iId);
  void RemoveProject(const QString& sName);
  void RenameProject(qint32 iId, const QString& sNewName);
  void RenameProject(const QString& sName, const QString& sNewName);
  bool SerializeProject(qint32 iId, bool bForceWriting = false);
  bool SerializeProject(const QString& sName, bool bForceWriting = false);

  // Scene
  static qint32 AddScene(tspProject& spProj, const QString& sName = "New_Scene",
                  const tvfnActionsScene& vfnActionsAfterAdding = tvfnActionsScene());
  static void ClearScenes(tspProject& spProj);
  static tspScene FindScene(const tspProject& spProj, qint32 iId);
  static tspScene FindScene(const tspProject& spProj, const QString& sName);
  static void RemoveScene(tspProject& spProj, qint32 iId);
  static void RemoveScene(tspProject& spProj, const QString& sName);
  static void RenameScene(tspProject& spProj, qint32 iId, const QString& sNewName);
  static void RenameScene(tspProject& spProj, const QString& sName, const QString& sNewName);

  // Resource
  static bool AddResourceArchive(tspProject& spProj, const SResourcePath& sPath);
  static QString AddResource(tspProject& spProj, const SResourcePath& sPath, const EResourceType& type,
                             const QString& sName = QString(), const QString& sBundle = QString(),
                             const tvfnActionsResource& vfnActionsAfterAdding = tvfnActionsResource());
  static void ClearResources(tspProject& spProj);
  static tspResource FindResourceInProject(const tspProject& spProj, const QString& sName);
  static tvspResource FindResourcesInProject(const tspProject& spProj, const QRegExp& rx);
  static tspResourceBundle FindResourceBundleInProject(const tspProject& spProj, const QString& sName);
  static void RemoveResource(tspProject& spProj, const QString& sName);
  static void RenameResource(tspProject& spProj, const QString& sName, const QString& sNewName);

  // Tags
  static QString AddTag(tspProject& spProj, const QString& sResource, const QString& sCategory,
                        const QString& sName, const QString& sDescribtion);
  static void ClearTags(tspProject& spProj);
  static tspTag FindTagInProject(const tspProject& spProj, QString sName);
  static void RemoveTag(tspProject& spProj, const QString& sName);
  static void RemoveTagFromResource(tspProject& spProj, const QString& sResource, const QString& sName);
  static QStringList TagCategories(const tspProject& spProj);

  // Kinks
  tspKink FindKink(QString sName);
  tspKink FindKink(QString sCategory, QString sName);
  QStringList FindKinks(QString sCategory);
  QStringList KinkCategories();

  // Achievements
  static QString AddAchievement(tspProject& spProj, const QString& sName, const QString& sDescribtion,
                                qint32 iType, const QString& sResource, const QVariant& data);
  static void ClearAchievement(tspProject& spProj);
  static tspSaveData FindAchievementInProject(const tspProject& spProj, QString sName);
  static void RemoveAchievement(tspProject& spProj, const QString& sName);
  static void RenameAchievement(tspProject& spProj, const QString& sName, const QString& sNewName);

  qint32 FindNewProjectId();
  bool IsDbLoaded() const;

public slots:
  void Initialize() override;
  void Deinitialize() override;

signals:
  void SignalProjectAdded(qint32 iId);
  void SignalProjectRenamed(qint32 iId);
  void SignalProjectRemoved(qint32 iId);
  void SignalSceneAdded(qint32 iProjId, qint32 iId);
  void SignalSceneDataChanged(qint32 iProjId, qint32 iId);
  void SignalSceneRenamed(qint32 iProjId, qint32 iId);
  void SignalSceneRemoved(qint32 iProjId, qint32 iId);
  void SignalReloadFinished();
  void SignalReloadStarted();
  void SignalResourceAdded(qint32 iProjId, const QString& sName);
  void SignalResourceRenamed(qint32 iProjId, const QString& sOldName, const QString& sName);
  void SignalResourceRemoved(qint32 iProjId, const QString& sName);
  void SignalTagAdded(qint32 iProjId, const QString& sResource, const QString& sName);
  void SignalTagRemoved(qint32 iProjId, const QString& sResource, const QString& sName);
  void SignalAchievementAdded(qint32 iProjId, const QString& sName);
  void SignalAchievementRemoved(qint32 iProjId, const QString& sName);
  void SignalAchievementDataChanged(qint32 iProjId, const QString& sName);
  void SignalAchievementRenamed(qint32 iProjId, const QString& sOldName, const QString& sName);

private slots:
  void SlotContentFolderChanged();

protected:
  qint32 AddProjectPrivate(const QString& sName,
                           const QString& sDirNameResolved,
                           const QString& sProjectPath,
                           qint32 iNewId,
                           quint32 iVersion,
                           bool bBundled, bool bReadOnly,
                           const tvfnActionsProject& vfnActionsAfterAdding);
  qint32 FindNewIdFromSet(const std::set<qint32, std::less<qint32>>& ids);
  qint32 FindNewSceneId(tspProject& spProj);
  static void RemoveLingeringTagReferencesFromResources(tspProject& spProj,
                                                        tspTag& spTag);

  std::unique_ptr<CDatabaseIO>           m_spDbIo;
  std::shared_ptr<CSettings>             m_spSettings;
  std::shared_ptr<CDatabaseData>         m_spData;
};

#endif // DATABASEMANAGER_H
