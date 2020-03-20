#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include "ThreadedSystem.h"
#include "Resource.h"
#include <QMutex>
#include <set>

class CProject;
class CScene;
class CSettings;
struct SProject;
struct SScene;
typedef std::shared_ptr<SProject> tspProject;
typedef std::vector<tspProject>   tvspProject;
typedef std::shared_ptr<SScene>   tspScene;

class CDatabaseManager : public CSystemBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CDatabaseManager)

public:
  CDatabaseManager();
  ~CDatabaseManager() override;

  // Project
  qint32 AddProject(const QString& sName = "New_Project", qint32 iVersion = 1);
  void ClearProjects();
  bool DeserializeProject(qint32 iId);
  bool DeserializeProject(const QString& sName);
  tspProject FindProject(qint32 iId);
  tspProject FindProject(const QString& sName);
  std::set<qint32, std::less<qint32>> ProjectIds();
  void RemoveProject(qint32 iId);
  void RemoveProject(const QString& sName);
  void RenameProject(qint32 iId, const QString& sNewName);
  void RenameProject(const QString& sName, const QString& sNewName);
  bool SerializeProject(qint32 iId);
  bool SerializeProject(const QString& sName);

  // Scene
  qint32 AddScene(tspProject& spProj, const QString& sName = "New_Scene");
  void ClearScenes(tspProject& spProj);
  tspScene FindScene(tspProject& spProj, qint32 iId);
  tspScene FindScene(tspProject& spProj, const QString& sName);
  void RemoveScene(tspProject& spProj, qint32 iId);
  void RemoveScene(tspProject& spProj, const QString& sName);
  void RenameScene(tspProject& spProj, qint32 iId, const QString& sNewName);
  void RenameScene(tspProject& spProj, const QString& sName, const QString& sNewName);

  // Resource
  QString AddResource(tspProject& spProj, const QUrl& sPath, const EResourceType& type, const QString& sName = QString());
  void ClearResources(tspProject& spProj);
  tspResource FindResource(tspProject& spProj, const QString& sName);
  void RemoveResource(tspProject& spProj, const QString& sName);
  void RenameResource(tspProject& spProj, const QString& sName, const QString& sNewName);

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
  bool SerializeProjectPrivate(tspProject& spProject);

  std::shared_ptr<CSettings>             m_spSettings;
  mutable QMutex                         m_dbMutex;
  tvspProject                            m_vspProjectDatabase;
};

#endif // DATABASEMANAGER_H
