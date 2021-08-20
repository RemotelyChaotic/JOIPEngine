#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include "ThreadedSystem.h"
#include "Kink.h"
#include "Resource.h"
#include <QAtomicInt>
#include <QMutex>
#include <set>

class CProjectScriptWrapper;
class CSceneScriptWrapper;
class CSettings;
struct SProject;
struct SScene;
typedef std::shared_ptr<SProject> tspProject;
typedef std::vector<tspProject>   tvspProject;
typedef std::shared_ptr<SScene>   tspScene;

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

  static bool LoadProject(tspProject& spProject);
  static bool UnloadProject(tspProject& spProject);

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
  bool SerializeProject(qint32 iId);
  bool SerializeProject(const QString& sName);

  // Scene
  qint32 AddScene(tspProject& spProj, const QString& sName = "New_Scene",
                  const tvfnActionsScene& vfnActionsAfterAdding = tvfnActionsScene());
  void ClearScenes(tspProject& spProj);
  tspScene FindScene(tspProject& spProj, qint32 iId);
  tspScene FindScene(tspProject& spProj, const QString& sName);
  void RemoveScene(tspProject& spProj, qint32 iId);
  void RemoveScene(tspProject& spProj, const QString& sName);
  void RenameScene(tspProject& spProj, qint32 iId, const QString& sNewName);
  void RenameScene(tspProject& spProj, const QString& sName, const QString& sNewName);

  // Resource
  bool AddResourceArchive(tspProject& spProj, const QUrl& sPath);
  QString AddResource(tspProject& spProj, const QUrl& sPath, const EResourceType& type, const QString& sName = QString(),
                      const tvfnActionsResource& vfnActionsAfterAdding = tvfnActionsResource());
  void ClearResources(tspProject& spProj);
  tspResource FindResourceInProject(tspProject& spProj, const QString& sName);
  void RemoveResource(tspProject& spProj, const QString& sName);
  void RenameResource(tspProject& spProj, const QString& sName, const QString& sNewName);

  // Kinks
  tspKink FindKink(QString sName);
  tspKink FindKink(QString sCategory, QString sName);
  QStringList FindKinks(QString sCategory);
  QStringList KinkCategories();

  bool IsDbLoaded() { return m_bLoadedDb == 1; }

public slots:
  void Initialize() override;
  void Deinitialize() override;

signals:
  void SignalProjectAdded(qint32 iId);
  void SignalProjectRenamed(qint32 iId);
  void SignalProjectRemoved(qint32 iId);
  void SignalSceneAdded(qint32 iProjId, qint32 iId);
  void SignalSceneRenamed(qint32 iProjId, qint32 iId);
  void SignalSceneRemoved(qint32 iProjId, qint32 iId);
  void SignalReloadFinished();
  void SignalReloadStarted();
  void SignalResourceAdded(qint32 iProjId, const QString& sName);
  void SignalResourceRenamed(qint32 iProjId, const QString& sOldName, const QString& sName);
  void SignalResourceRemoved(qint32 iProjId, const QString& sName);

private slots:
  void SlotContentFolderChanged();

private:
  bool DeserializeProjectPrivate(tspProject& spProject);
  qint32 FindNewIdFromSet(const std::set<qint32, std::less<qint32>>& ids);
  qint32 FindNewProjectId();
  qint32 FindNewSceneId(tspProject& spProj);
  void LoadDatabase();
  bool PrepareProjectPrivate(tspProject& spProject);
  bool SerializeProjectPrivate(tspProject& spProject);

  static bool MountProject(tspProject& spProject);
  static bool UnmountProject(tspProject& spProject);

  std::shared_ptr<CSettings>             m_spSettings;
  mutable QMutex                         m_dbMutex;
  QAtomicInt                             m_bLoadedDb;
  tvspProject                            m_vspProjectDatabase;
  tKinkKategories                        m_kinkKategoryMap;
};

#endif // DATABASEMANAGER_H
