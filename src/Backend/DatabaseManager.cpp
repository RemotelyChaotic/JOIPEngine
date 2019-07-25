#include "DatabaseManager.h"
#include "Application.h"
#include "Project.h"
#include "Resource.h"
#include "Scene.h"
#include "Settings.h"
#include <QDebug>
#include <QFileInfo>

CDatabaseManager::CDatabaseManager() :
  CThreadedObject(),
  m_spSettings(CApplication::Instance()->Settings()),
  m_vspProjectDatabase()
{
}

CDatabaseManager::~CDatabaseManager()
{
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::AddProject(const QString& sName, qint32 iVersion)
{
  qint32 iNewId = FindNewProjectId();
  QString sFinalName = sName;
  qint32 iCounter = 0;
  while (FindProject(sFinalName) != nullptr)
  {
    sFinalName = sName + QString::number(iCounter);
    iCounter++;
  }

  QMutexLocker locker(&m_dbMutex);
  if (0 <= iNewId)
  {
    m_vspProjectDatabase.push_back(std::make_shared<SProject>());
    m_vspProjectDatabase.back()->m_iId = iNewId;
    m_vspProjectDatabase.back()->m_sName = sFinalName;
    m_vspProjectDatabase.back()->m_iVersion = iVersion;
  }
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::ClearProjects()
{
  QMutexLocker locker(&m_dbMutex);
  m_vspProjectDatabase.clear();
}

//----------------------------------------------------------------------------------------
//
tspProject CDatabaseManager::FindProject(qint32 iId)
{
  QMutexLocker locker(&m_dbMutex);
  for (tspProject& spProject : m_vspProjectDatabase)
  {
    QReadLocker projLocker(&spProject->m_rwLock);
    if (spProject->m_iId == iId)
    {
      return spProject;
    }
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
tspProject CDatabaseManager::FindProject(const QString& sName)
{
  QMutexLocker locker(&m_dbMutex);
  for (tspProject& spProject : m_vspProjectDatabase)
  {
    QReadLocker projLocker(&spProject->m_rwLock);
    if (spProject->m_sName == sName)
    {
      return spProject;
    }
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
tspProjectRef CDatabaseManager::FindProjectRef(qint32 iId)
{
  QMutexLocker locker(&m_dbMutex);
  for (tspProject& spProject : m_vspProjectDatabase)
  {
    QReadLocker projLocker(&spProject->m_rwLock);
    if (spProject->m_iId == iId)
    {
      return tspProjectRef(new CProject(spProject));
    }
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
tspProjectRef CDatabaseManager::FindProjectRef(const QString& sName)
{
  QMutexLocker locker(&m_dbMutex);
  for (tspProject& spProject : m_vspProjectDatabase)
  {
    QReadLocker projLocker(&spProject->m_rwLock);
    if (spProject->m_sName == sName)
    {
      return tspProjectRef(new CProject(spProject));
    }
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::RemoveProject(qint32 iId)
{
  QMutexLocker locker(&m_dbMutex);
  for (auto it = m_vspProjectDatabase.begin(); m_vspProjectDatabase.end() != it; ++it)
  {
    QReadLocker projLocker(&(*it)->m_rwLock);
    if ((*it)->m_iId == iId)
    {
      m_vspProjectDatabase.erase(it);
      break;
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::RemoveProject(const QString& sName)
{
  QMutexLocker locker(&m_dbMutex);
  for (auto it = m_vspProjectDatabase.begin(); m_vspProjectDatabase.end() != it; ++it)
  {
    QReadLocker projLocker(&(*it)->m_rwLock);
    if ((*it)->m_sName == sName)
    {
      m_vspProjectDatabase.erase(it);
      break;
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::RenameProject(qint32 iId, const QString& sNewName)
{
  tspProject spNewProject = FindProject(sNewName);
  if (nullptr == spNewProject)
  {
    tspProject spProject = FindProject(iId);
    spProject->m_rwLock.lockForWrite();
    spProject->m_sName = sNewName;
    spProject->m_rwLock.unlock();
  }
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::RenameProject(const QString& sName, const QString& sNewName)
{
  tspProject spNewProject = FindProject(sNewName);
  if (nullptr == spNewProject)
  {
    tspProject spProject = FindProject(sName);
    spProject->m_rwLock.lockForWrite();
    spProject->m_sName = sNewName;
    spProject->m_rwLock.unlock();
  }
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::AddScene(tspProject& spProj, const QString& sName)
{
  qint32 iNewId = FindNewSceneId(spProj);
  QString sFinalName = sName;
  qint32 iCounter = 0;
  while (FindScene(spProj, sFinalName) != nullptr)
  {
    sFinalName = sName + QString::number(iCounter);
    iCounter++;
  }

  QWriteLocker locker(&spProj->m_rwLock);
  if (0 <= iNewId)
  {
    spProj->m_vspScenes.push_back(std::make_shared<SScene>());
    spProj->m_vspScenes.back()->m_iId = iNewId;
    spProj->m_vspScenes.back()->m_sName = sFinalName;
    spProj->m_vspScenes.back()->m_spParent = spProj;
  }
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::ClearScenes(tspProject& spProj)
{
  QWriteLocker locker(&spProj->m_rwLock);
  spProj->m_vspScenes.clear();
}

//----------------------------------------------------------------------------------------
//
tspScene CDatabaseManager::FindScene(tspProject& spProj, qint32 iId)
{
  QReadLocker locker(&spProj->m_rwLock);
  for (tspScene& spScene : spProj->m_vspScenes)
  {
    QReadLocker projLocker(&spScene->m_rwLock);
    if (spScene->m_iId == iId)
    {
      return spScene;
    }
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
tspScene CDatabaseManager::FindScene(tspProject& spProj, const QString& sName)
{
  QReadLocker locker(&spProj->m_rwLock);
  for (tspScene& spScene : spProj->m_vspScenes)
  {
    QReadLocker projLocker(&spScene->m_rwLock);
    if (spScene->m_sName == sName)
    {
      return spScene;
    }
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
tspSceneRef CDatabaseManager::FindSceneRef(tspProject& spProj, qint32 iId)
{
  QReadLocker locker(&spProj->m_rwLock);
  for (tspScene& spScene : spProj->m_vspScenes)
  {
    QReadLocker projLocker(&spScene->m_rwLock);
    if (spScene->m_iId == iId)
    {
      return tspSceneRef(new CScene(spScene));
    }
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
tspSceneRef CDatabaseManager::FindSceneRef(tspProject& spProj, const QString& sName)
{
  QReadLocker locker(&spProj->m_rwLock);
  for (tspScene& spScene : spProj->m_vspScenes)
  {
    QReadLocker projLocker(&spScene->m_rwLock);
    if (spScene->m_sName == sName)
    {
      return tspSceneRef(new CScene(spScene));
    }
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::RemoveScene(tspProject& spProj, qint32 iId)
{
  QWriteLocker locker(&spProj->m_rwLock);
  for (auto it = spProj->m_vspScenes.begin(); spProj->m_vspScenes.end() != it; ++it)
  {
    QReadLocker projLocker(&(*it)->m_rwLock);
    if ((*it)->m_iId == iId)
    {
      spProj->m_vspScenes.erase(it);
      break;
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::RemoveScene(tspProject& spProj, const QString& sName)
{
  QWriteLocker locker(&spProj->m_rwLock);
  for (auto it = spProj->m_vspScenes.begin(); spProj->m_vspScenes.end() != it; ++it)
  {
    QReadLocker projLocker(&(*it)->m_rwLock);
    if ((*it)->m_sName == sName)
    {
      spProj->m_vspScenes.erase(it);
      break;
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::RenameScene(tspProject& spProj, qint32 iId, const QString& sNewName)
{
  tspScene spNewScene = FindScene(spProj, sNewName);
  if (nullptr == spNewScene)
  {
    tspScene spScene = FindScene(spProj, iId);
    spScene->m_rwLock.lockForWrite();
    spScene->m_sName = sNewName;
    spScene->m_rwLock.unlock();
  }
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::RenameScene(tspProject& spProj, const QString& sName, const QString& sNewName)
{
  tspScene spNewScene = FindScene(spProj, sNewName);
  if (nullptr == spNewScene)
  {
    tspScene spScene = FindScene(spProj, sName);
    spScene->m_rwLock.lockForWrite();
    spScene->m_sName = sNewName;
    spScene->m_rwLock.unlock();
  }
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::AddResource(tspProject& spProj, const QString& sPath, const EResourceType& type, const QString& sName)
{
  QString sFinalName = sName;
  if (sName.isNull())
  {
    QFileInfo info(sPath);
    sFinalName = info.fileName();
    qint32 iCounter = 0;
    while (FindResource(spProj, sFinalName) != nullptr)
    {
      sFinalName = info.baseName() + QString::number(iCounter) + info.fileName().replace(info.baseName(), "");
      iCounter++;
    }
  }

  QWriteLocker locker(&spProj->m_rwLock);
  std::shared_ptr<SResource> sResource = std::make_shared<SResource>();
  sResource->m_sPath = sPath;
  sResource->m_type = type;
  spProj->m_spResourcesMap.insert({sFinalName, sResource});
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::ClearResources(tspProject& spProj)
{
  QWriteLocker locker(&spProj->m_rwLock);
  spProj->m_vspScenes.clear();
  for (tspScene& spScene : spProj->m_vspScenes)
  {
    QWriteLocker sceneLocker(&spScene->m_rwLock);
    spScene->m_vsResourceRefs.clear();
  }
}

//----------------------------------------------------------------------------------------
//
tspResource CDatabaseManager::FindResource(tspProject& spProj, const QString& sName)
{
  QReadLocker locker(&spProj->m_rwLock);
  auto it = spProj->m_spResourcesMap.find(sName);
  if (it != spProj->m_spResourcesMap.end())
  {
    return it->second;
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
tspResourceRef CDatabaseManager::FindResourceRef(tspProject& spProj, const QString& sName)
{
  QReadLocker locker(&spProj->m_rwLock);
  auto it = spProj->m_spResourcesMap.find(sName);
  if (it != spProj->m_spResourcesMap.end())
  {
    return tspResourceRef(new CResource(it->second));
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::RemoveResource(tspProject& spProj, const QString& sName)
{
  QWriteLocker locker(&spProj->m_rwLock);
  auto it = spProj->m_spResourcesMap.find(sName);
  if (it != spProj->m_spResourcesMap.end())
  {
    spProj->m_spResourcesMap.erase(it);
    for (tspScene& spScene : spProj->m_vspScenes)
    {
      QWriteLocker sceneLocker(&spScene->m_rwLock);
      auto refsIt = spScene->m_vsResourceRefs.find(sName);
      if (refsIt != spScene->m_vsResourceRefs.end())
      {
        spScene->m_vsResourceRefs.erase(refsIt);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::RenameResource(tspProject& spProj, const QString& sName, const QString& sNewName)
{
  tspResource spNewResource = FindResource(spProj, sNewName);
  if (nullptr == spNewResource)
  {
    tspResource spResource = FindResource(spProj, sName);

    QWriteLocker locker(&spProj->m_rwLock);
    auto it = spProj->m_spResourcesMap.find(sName);
    if (it != spProj->m_spResourcesMap.end())
    {
      spProj->m_spResourcesMap.erase(it);
      spProj->m_spResourcesMap.insert({sNewName, spResource});
      for (tspScene& spScene : spProj->m_vspScenes)
      {
        QWriteLocker sceneLocker(&spScene->m_rwLock);
        auto refsIt = spScene->m_vsResourceRefs.find(sName);
        if (refsIt != spScene->m_vsResourceRefs.end())
        {
          spScene->m_vsResourceRefs.erase(refsIt);
          spScene->m_vsResourceRefs.insert(sNewName);
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::Initialize()
{
  connect(m_spSettings.get(), &CSettings::ContentFolderChanged,
          this, &CDatabaseManager::SlotContentFolderChanged, Qt::QueuedConnection);

  SlotContentFolderChanged();

  SetInitialized(true);
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::Deinitialize()
{
  disconnect(m_spSettings.get(), &CSettings::ContentFolderChanged,
          this, &CDatabaseManager::SlotContentFolderChanged);

  SlotContentFolderChanged();

  SetInitialized(false);
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::SlotContentFolderChanged()
{
  ClearProjects();
  LoadDatabase();
}

//----------------------------------------------------------------------------------------
//
qint32 CDatabaseManager::FindNewIdFromSet(const std::set<qint32, std::less<qint32>>& ids)
{
  qint32 iNewId = 0;
  for (auto it = ids.begin(); ids.end() != it; ++it)
  {
    if (*it == iNewId)
    {
      iNewId++;
    }
  }
  return iNewId;
}

//----------------------------------------------------------------------------------------
//
qint32 CDatabaseManager::FindNewProjectId()
{
  QMutexLocker locker(&m_dbMutex);
  std::set<qint32, std::less<qint32>> ids;
  for (tspProject& spProject : m_vspProjectDatabase)
  {
    QReadLocker projLocker(&spProject->m_rwLock);
    ids.insert(spProject->m_iId);
  }
  locker.unlock();
  return FindNewIdFromSet(ids);
}

//----------------------------------------------------------------------------------------
//
qint32 CDatabaseManager::FindNewSceneId(tspProject& spProj)
{
  QReadLocker projLocker(&spProj->m_rwLock);
  std::set<qint32, std::less<qint32>> ids;
  for (tspScene& spScene : spProj->m_vspScenes)
  {
    QReadLocker sceneLocker(&spScene->m_rwLock);
    ids.insert(spScene->m_iId);
  }
  projLocker.unlock();
  return FindNewIdFromSet(ids);
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::LoadDatabase()
{
  // TODO:
}
