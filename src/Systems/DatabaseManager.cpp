#include "DatabaseManager.h"
#include "Application.h"
#include "DatabaseData.h"
#include "DatabaseIO.h"
#include "Project.h"
#include "Resource.h"
#include "Scene.h"
#include "Settings.h"

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QResource>

//----------------------------------------------------------------------------------------
//
CDatabaseManager::CDatabaseManager() :
  CSystemBase(),
  m_spDbIo(),
  m_spSettings(CApplication::Instance()->Settings()),
  m_spData(std::make_shared<CDatabaseData>())
{
  m_spDbIo = CDatabaseIO::CreateDatabaseIO(this, m_spData);
}

CDatabaseManager::~CDatabaseManager()
{
}

//----------------------------------------------------------------------------------------
//
bool CDatabaseManager::LoadBundle(tspProject& spProject, const QString& sBundle)
{
  return CDatabaseIO::LoadBundle(spProject, sBundle);
}

//----------------------------------------------------------------------------------------
//
bool CDatabaseManager::LoadProject(tspProject& spProject)
{
  return CDatabaseIO::LoadProject(spProject);
}

//----------------------------------------------------------------------------------------
//
bool CDatabaseManager::SetProjectEditing(tspProject& spProject, bool bEnabled)
{
  return CDatabaseIO::SetProjectEditing(spProject, bEnabled);
}

//----------------------------------------------------------------------------------------
//
bool CDatabaseManager::UnloadBundle(tspProject& spProject, const QString& sBundle)
{
  return CDatabaseIO::UnloadBundle(spProject, sBundle);
}

//----------------------------------------------------------------------------------------
//
bool CDatabaseManager::UnloadProject(tspProject& spProject)
{
  return CDatabaseIO::UnloadProject(spProject);
}

//----------------------------------------------------------------------------------------
//
qint32 CDatabaseManager::AddProject(const QDir& dir, quint32 iVersion,
                                    bool bBundled, bool bReadOnly,
                                    const tvfnActionsProject& vfnActionsAfterAdding)
{
  if (!IsInitialized()) { return -1; }

  qint32 iNewId = FindNewProjectId();
  const QString sBaseName = dir.dirName();
  const QString sProjectPath = QFileInfo(dir.absolutePath()).absolutePath();
  QString sName = sBaseName;
  QString sDirNameResolved;
  QString sError;
  if (!ProjectNameCheck(sBaseName, &sError))
  {
    sName = ToValidProjectName(sBaseName);
    sDirNameResolved = sName;
  }
  else
  {
    sName = sBaseName;
    sDirNameResolved = sBaseName;
  }

  return AddProjectPrivate(sName, sDirNameResolved, sProjectPath,
                           iNewId, iVersion,
                           bBundled, bReadOnly,
                           vfnActionsAfterAdding);
}

//----------------------------------------------------------------------------------------
//
qint32 CDatabaseManager::AddProject(const QString& sDirName, quint32 iVersion,
                                    bool bBundled, bool bReadOnly,
                                    const tvfnActionsProject& vfnActionsAfterAdding)
{
  if (!IsInitialized()) { return -1; }

  qint32 iNewId = FindNewProjectId();
  const QString sBaseName = QFileInfo(sDirName).completeBaseName();
  const QString sProjectPath = QFileInfo(sDirName).absolutePath();
  QString sName = sBaseName;
  QString sDirNameResolved = sBaseName + "." + QFileInfo(sDirName).suffix();
  QString sError;
  if (!ProjectNameCheck(sBaseName, &sError))
  {
    sName = ToValidProjectName(sBaseName);
  }
  else
  {
    sName = sBaseName;
  }

  return AddProjectPrivate(sName, sDirNameResolved, sProjectPath,
                           iNewId, iVersion,
                           bBundled, bReadOnly,
                           vfnActionsAfterAdding);
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::ClearProjects()
{
  if (!IsInitialized()) { return; }

  QMutexLocker locker(m_spData.get());
  while (0 < m_spData->m_vspProjectDatabase.size())
  {
    auto it = m_spData->m_vspProjectDatabase.begin();
    QReadLocker projLocker(&(*it)->m_rwLock);
    qint32 iId = (*it)->m_iId;
    QString sName = (*it)->m_sName;
    projLocker.unlock();

    UnloadProject(*it);

    m_spData->m_vspProjectDatabase.erase(it);

    locker.unlock();

    emit SignalProjectRemoved(iId);
    locker.relock();
  }

  m_spDbIo->SetDbLoaded(false);
}

//----------------------------------------------------------------------------------------
//
bool CDatabaseManager::DeserializeProject(qint32 iId)
{
  if (!IsInitialized()) { return false; }

  tspProject spProject = FindProject(iId);
  if (nullptr != spProject)
  {
    return m_spDbIo->DeserializeProject(spProject);
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
bool CDatabaseManager::DeserializeProject(const QString& sName)
{
  if (!IsInitialized()) { return false; }

  tspProject spProject = FindProject(sName);
  if (nullptr != spProject)
  {
    return m_spDbIo->DeserializeProject(spProject);
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
tspProject CDatabaseManager::FindProject(qint32 iId)
{
  if (!IsInitialized()) { return nullptr; }

  QMutexLocker locker(m_spData.get());
  for (tspProject& spProject : m_spData->m_vspProjectDatabase)
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
  if (!IsInitialized()) { return nullptr; }

  QMutexLocker locker(m_spData.get());
  for (tspProject& spProject : m_spData->m_vspProjectDatabase)
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
std::set<qint32, std::less<qint32>> CDatabaseManager::ProjectIds()
{
  if (!IsInitialized()) { return std::set<qint32>(); }

  QMutexLocker locker(m_spData.get());
  std::set<qint32, std::less<qint32>> ids;
  for (tspProject& spProject : m_spData->m_vspProjectDatabase)
  {
    QReadLocker projLocker(&spProject->m_rwLock);
    ids.insert(spProject->m_iId);
  }
  return ids;
}

//----------------------------------------------------------------------------------------
//
bool CDatabaseManager::PrepareNewProject(qint32 iId)
{
  if (!IsInitialized()) { return false; }

  tspProject spProject = FindProject(iId);
  if (nullptr != spProject)
  {
    return m_spDbIo->PrepareProject(spProject);
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
bool CDatabaseManager::PrepareNewProject(const QString& sName)
{
  if (!IsInitialized()) { return false; }

  tspProject spProject = FindProject(sName);
  if (nullptr != spProject)
  {
    return m_spDbIo->PrepareProject(spProject);
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::RemoveProject(qint32 iId)
{
  if (!IsInitialized()) { return; }

  QMutexLocker locker(m_spData.get());
  for (auto it = m_spData->m_vspProjectDatabase.begin(); m_spData->m_vspProjectDatabase.end() != it; ++it)
  {
    QReadLocker projLocker(&(*it)->m_rwLock);
    QString sName = (*it)->m_sName;
    qint32 iFoundId = (*it)->m_iId;
    projLocker.unlock();
    if (iFoundId == iId)
    {
      UnloadProject(*it);

      m_spData->m_vspProjectDatabase.erase(it);

      locker.unlock();

      emit SignalProjectRemoved(iFoundId);
      break;
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::RemoveProject(const QString& sName)
{
  if (!IsInitialized()) { return; }

  QMutexLocker locker(m_spData.get());
  for (auto it = m_spData->m_vspProjectDatabase.begin(); m_spData->m_vspProjectDatabase.end() != it; ++it)
  {
    QReadLocker projLocker(&(*it)->m_rwLock);
    qint32 iFoundId = (*it)->m_iId;
    projLocker.unlock();
    if ((*it)->m_sName == sName)
    {
      UnloadProject(*it);

      m_spData->m_vspProjectDatabase.erase(it);

      locker.unlock();

      emit SignalProjectRemoved(iFoundId);
      break;
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::RenameProject(qint32 iId, const QString& sNewName)
{
  if (!IsInitialized()) { return; }

  tspProject spNewProject = FindProject(sNewName);
  if (nullptr == spNewProject && ProjectNameCheck(sNewName))
  {
    tspProject spProject = FindProject(iId);

    QReadLocker locker(&spProject->m_rwLock);
    bool bLoadedBefore = spProject->m_bLoaded;
    locker.unlock();

    UnloadProject(spProject);

    spProject->m_rwLock.lockForWrite();
    spProject->m_sName = sNewName;
    spProject->m_rwLock.unlock();

    if (bLoadedBefore) { LoadProject(spProject); }

    emit SignalProjectRenamed(iId);
  }
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::RenameProject(const QString& sName, const QString& sNewName)
{
  if (!IsInitialized()) { return; }

  tspProject spNewProject = FindProject(sNewName);
  if (nullptr == spNewProject && ProjectNameCheck(sNewName))
  {
    tspProject spProject = FindProject(sName);

    QReadLocker locker(&spProject->m_rwLock);
    bool bLoadedBefore = spProject->m_bLoaded;
    locker.unlock();

    UnloadProject(spProject);

    spProject->m_rwLock.lockForWrite();
    qint32 iId = spProject->m_iId;
    spProject->m_sName = sNewName;
    spProject->m_rwLock.unlock();

    if (bLoadedBefore) { LoadProject(spProject); }

    emit SignalProjectRenamed(iId);
  }
}

//----------------------------------------------------------------------------------------
//
bool CDatabaseManager::SerializeProject(qint32 iId, bool bForceWriting)
{
  if (!IsInitialized()) { return false; }

  tspProject spProject = FindProject(iId);
  if (nullptr != spProject)
  {
    return m_spDbIo->SerializeProject(spProject, bForceWriting);
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
bool CDatabaseManager::SerializeProject(const QString& sName, bool bForceWriting)
{
  if (!IsInitialized()) { return false; }

  tspProject spProject = FindProject(sName);
  if (nullptr != spProject)
  {
    return m_spDbIo->SerializeProject(spProject, bForceWriting);
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
qint32 CDatabaseManager::AddScene(tspProject& spProj, const QString& sName,
                                  const tvfnActionsScene& vfnActionsAfterAdding)
{
  if (!IsInitialized() || nullptr == spProj) { return -1; }

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

    locker.unlock();

    for (auto fn : vfnActionsAfterAdding)
    {
      if (nullptr != fn) { fn(spProj->m_vspScenes.back()); }
    }

    emit SignalSceneAdded(spProj->m_iId, iNewId);
  }
  return iNewId;
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::ClearScenes(tspProject& spProj)
{
  if (!IsInitialized() || nullptr == spProj) { return; }

  QWriteLocker locker(&spProj->m_rwLock);
  while (0 < spProj->m_vspScenes.size())
  {
    auto it = spProj->m_vspScenes.begin();
    QReadLocker sceneLocker(&(*it)->m_rwLock);
    qint32 iSceneId = (*it)->m_iId;
    sceneLocker.unlock();
    spProj->m_vspScenes.erase(it);

    locker.unlock();
    emit SignalSceneRemoved(spProj->m_iId, iSceneId);
    locker.relock();
  }
}

//----------------------------------------------------------------------------------------
//
tspScene CDatabaseManager::FindScene(tspProject& spProj, qint32 iId)
{
  if (!IsInitialized() || nullptr == spProj) { return nullptr; }

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
  if (!IsInitialized() || nullptr == spProj) { return nullptr; }

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
void CDatabaseManager::RemoveScene(tspProject& spProj, qint32 iId)
{
  if (!IsInitialized() || nullptr == spProj) { return; }

  QWriteLocker locker(&spProj->m_rwLock);
  for (auto it = spProj->m_vspScenes.begin(); spProj->m_vspScenes.end() != it; ++it)
  {
    QReadLocker sceneLocker(&(*it)->m_rwLock);
    qint32 iSceneId = (*it)->m_iId;
    sceneLocker.unlock();
    if (iSceneId == iId)
    {
      spProj->m_vspScenes.erase(it);

      locker.unlock();
      emit SignalSceneRemoved(spProj->m_iId, iSceneId);
      break;
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::RemoveScene(tspProject& spProj, const QString& sName)
{
  if (!IsInitialized() || nullptr == spProj) { return; }

  QWriteLocker locker(&spProj->m_rwLock);
  for (auto it = spProj->m_vspScenes.begin(); spProj->m_vspScenes.end() != it; ++it)
  {
    QReadLocker sceneLocker(&(*it)->m_rwLock);
    qint32 iSceneId = (*it)->m_iId;
    const QString sFoundName = (*it)->m_sName;
    sceneLocker.unlock();
    if (sFoundName == sName)
    {
      spProj->m_vspScenes.erase(it);

      locker.unlock();
      emit SignalSceneRemoved(spProj->m_iId, iSceneId);
      break;
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::RenameScene(tspProject& spProj, qint32 iId, const QString& sNewName)
{
  if (!IsInitialized() || nullptr == spProj) { return; }

  tspScene spNewScene = FindScene(spProj, sNewName);
  if (nullptr == spNewScene)
  {
    spProj->m_rwLock.lockForRead();
    qint32 iProjId = spProj->m_iId;
    spProj->m_rwLock.unlock();

    tspScene spScene = FindScene(spProj, iId);
    spScene->m_rwLock.lockForWrite();
    spScene->m_sName = sNewName;
    spScene->m_rwLock.unlock();

    emit SignalSceneRenamed(iProjId, iId);
  }
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::RenameScene(tspProject& spProj, const QString& sName, const QString& sNewName)
{
  if (!IsInitialized() || nullptr == spProj) { return; }

  tspScene spNewScene = FindScene(spProj, sNewName);
  if (nullptr == spNewScene)
  {
    spProj->m_rwLock.lockForRead();
    qint32 iProjId = spProj->m_iId;
    spProj->m_rwLock.unlock();

    tspScene spScene = FindScene(spProj, sName);
    spScene->m_rwLock.lockForWrite();
    spScene->m_sName = sNewName;
    qint32 iId = spScene->m_iId;
    spScene->m_rwLock.unlock();

    emit SignalSceneRenamed(iProjId, iId);
  }
}

//----------------------------------------------------------------------------------------
//
bool CDatabaseManager::AddResourceArchive(tspProject& spProj, const QUrl& sPath)
{
  if (!IsInitialized() || nullptr == spProj) { return false; }
  return m_spDbIo->AddResourceArchive(spProj, sPath);
}

//----------------------------------------------------------------------------------------
//
QString CDatabaseManager::AddResource(tspProject& spProj, const QUrl& sPath,
                                      const EResourceType& type, const QString& sName,
                                      const tvfnActionsResource& vfnActionsAfterAdding)
{
  if (!IsInitialized() || nullptr == spProj) { return QString(); }

  QString sFinalName = sName;
  if (sName.isNull())
  {
    sFinalName = sPath.fileName();
    QFileInfo info(sFinalName);
    qint32 iCounter = 0;
    while (FindResourceInProject(spProj, sFinalName) != nullptr)
    {
      sFinalName = info.baseName() + QString::number(iCounter) + sPath.fileName().replace(info.baseName(), "");
      iCounter++;
    }
  }

  QWriteLocker locker(&spProj->m_rwLock);
  if (!IsLocalFile(sPath))
  {
    spProj->m_bUsesWeb = true;
  }
  if (type._to_integral() == EResourceType::eMovie ||
      type._to_integral() == EResourceType::eSound)
  {
    spProj->m_bNeedsCodecs = true;
  }
  std::shared_ptr<SResource> spResource = std::make_shared<SResource>();
  spResource->m_sName = sFinalName;
  spResource->m_sPath = sPath;
  spResource->m_type = type;
  spResource->m_spParent = spProj;
  spProj->m_spResourcesMap.insert({sFinalName, spResource});

  locker.unlock();
  if (spProj->m_bLoaded)
  {
    m_spDbIo->LoadResource(spResource);
  }


  for (auto fn : vfnActionsAfterAdding)
  {
    if (nullptr != fn) { fn(spResource); }
  }

  emit SignalResourceAdded(spProj->m_iId, sFinalName);

  return sFinalName;
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::ClearResources(tspProject& spProj)
{
  if (!IsInitialized() || nullptr == spProj) { return; }

  QWriteLocker locker(&spProj->m_rwLock);
  while (0 < spProj->m_spResourcesMap.size())
  {
    auto it = spProj->m_spResourcesMap.begin();
    QString sName = it->first;

    if (spProj->m_bLoaded)
    {
      m_spDbIo->UnloadResource(it->second);
    }

    spProj->m_spResourcesMap.erase(it);

    for (tspScene& spScene : spProj->m_vspScenes)
    {
      QWriteLocker sceneLocker(&spScene->m_rwLock);
      spScene->m_vsResourceRefs.clear();
      if (spScene->m_sScript == sName)
      {
        spScene->m_sScript = QString();
      }
    }

    locker.unlock();
    emit SignalResourceRemoved(spProj->m_iId, sName);
    locker.relock();
  }

  ClearTags(spProj);
}

//----------------------------------------------------------------------------------------
//
tspResource CDatabaseManager::FindResourceInProject(tspProject& spProj, const QString& sName)
{
  if (!IsInitialized() || nullptr == spProj) { return nullptr; }

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
tvspResource CDatabaseManager::FindResourcesInProject(tspProject& spProj, const QRegExp& rx)
{
  tvspResource retVal;
  if (!IsInitialized() || nullptr == spProj) { return retVal; }

  QReadLocker locker(&spProj->m_rwLock);
  for (auto it = spProj->m_spResourcesMap.begin(); spProj->m_spResourcesMap.end() != it; ++it)
  {
    qint32 iPos = 0;
    if ((iPos = rx.indexIn(it->first, iPos)) == -1)
    {
      retVal.push_back(it->second);
    }
  }
  return retVal;
}

//----------------------------------------------------------------------------------------
//
tspResourceBundle CDatabaseManager::FindResourceBundleInProject(tspProject& spProj, const QString& sName)
{
  if (!IsInitialized() || nullptr == spProj) { return nullptr; }

  QReadLocker locker(&spProj->m_rwLock);
  auto it = spProj->m_spResourceBundleMap.find(sName);
  if (it != spProj->m_spResourceBundleMap.end())
  {
    return it->second;
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::RemoveResource(tspProject& spProj, const QString& sName)
{
  if (!IsInitialized() || nullptr == spProj) { return; }

  QWriteLocker locker(&spProj->m_rwLock);
  auto it = spProj->m_spResourcesMap.find(sName);
  if (it != spProj->m_spResourcesMap.end())
  {
    qint32 iId = spProj->m_iId;

    if (spProj->m_bLoaded)
    {
      m_spDbIo->UnloadResource(it->second);
    }

    spProj->m_spResourcesMap.erase(it);

    for (tspScene& spScene : spProj->m_vspScenes)
    {
      QWriteLocker sceneLocker(&spScene->m_rwLock);
      auto refsIt = spScene->m_vsResourceRefs.find(sName);
      if (refsIt != spScene->m_vsResourceRefs.end())
      {
        spScene->m_vsResourceRefs.erase(refsIt);
      }
      if (spScene->m_sScript == sName)
      {
        spScene->m_sScript = QString();
      }
    }

    // delete tag references
    for (auto itTag = spProj->m_vspTags.begin(); spProj->m_vspTags.end() != itTag;)
    {
      QWriteLocker tagLocker(&itTag->second->m_rwLock);
      QString sTagName = itTag->second->m_sName;
      auto itTagResRef = itTag->second->m_vsResourceRefs.find(sName);
      if (itTag->second->m_vsResourceRefs.end() != itTagResRef)
      {
        itTag->second->m_vsResourceRefs.erase(itTagResRef);
      }

      if (itTag->second->m_vsResourceRefs.empty())
      {
        tagLocker.unlock();
        itTag = spProj->m_vspTags.erase(itTag);
        locker.unlock();
        emit SignalTagRemoved(iId, QString(), sTagName);
      }
      else
      {
        ++itTag;
        tagLocker.unlock();
        locker.unlock();
        emit SignalTagRemoved(iId, sName, sTagName);
      }
    }

    // rename refs in achievements
    for (const auto& [sAchName, spAchievements] : spProj->m_vspAchievements)
    {
      if (sAchName == sName)
      {
        {
          QWriteLocker acLocker(&spAchievements->m_rwLock);
          spAchievements->m_sResource = QString();
        }
        emit SignalAchievementDataChanged(iId, sAchName);
      }
    }

    locker.unlock();
    emit SignalResourceRemoved(iId, sName);
  }
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::RenameResource(tspProject& spProj, const QString& sName, const QString& sNewName)
{
  if (!IsInitialized() || nullptr == spProj) { return; }

  tspResource spNewResource = FindResourceInProject(spProj, sNewName);
  if (nullptr == spNewResource)
  {
    tspResource spResource = FindResourceInProject(spProj, sName);

    QWriteLocker locker(&spProj->m_rwLock);
    auto it = spProj->m_spResourcesMap.find(sName);
    if (it != spProj->m_spResourcesMap.end())
    {
      qint32 iProjId = spProj->m_iId;

      // rename map and titlecard
      if (spProj->m_sMap == sName)
      {
        spProj->m_sMap = sNewName;
      }
      if (spProj->m_sTitleCard == sName)
      {
        spProj->m_sTitleCard = sNewName;
      }
      if (spProj->m_sSceneModel == sName)
      {
        spProj->m_sSceneModel = sNewName;
      }

      if (spProj->m_bLoaded)
      {
        m_spDbIo->UnloadResource(spResource);
      }

      spProj->m_spResourcesMap.erase(it);
      spResource->m_rwLock.lockForWrite();
      spResource->m_sName = sNewName;
      spResource->m_rwLock.unlock();
      spProj->m_spResourcesMap.insert({sNewName, spResource});

      if (spProj->m_bLoaded)
      {
        locker.unlock();
        m_spDbIo->LoadResource(spResource);
        locker.relock();
      }

      // rename refs from scene
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

      // rename refs in tags
      for (const auto& [_, spTag] : spProj->m_vspTags)
      {
        QWriteLocker tagLocker(&spTag->m_rwLock);
        auto itResRef = spTag->m_vsResourceRefs.find(sName);
        if (spTag->m_vsResourceRefs.end() != itResRef)
        {
          spTag->m_vsResourceRefs.erase(itResRef);
        }
        spTag->m_vsResourceRefs.insert(sNewName);
      }

      // rename refs in achievements
      for (const auto& [_, spAchievements] : spProj->m_vspAchievements)
      {
        QWriteLocker acLocker(&spAchievements->m_rwLock);
        spAchievements->m_sResource = sNewName;
      }

      locker.unlock();
      emit SignalResourceRenamed(iProjId, sName, sNewName);
    }
  }
}

//----------------------------------------------------------------------------------------
//
QString CDatabaseManager::AddTag(tspProject& spProj, const QString& sResource, const QString& sCategory,
                                 const QString& sName, const QString& sDescribtion)
{
  if (!IsInitialized() || nullptr == spProj || sName.isEmpty()) { return QString(); }

  bool bChanged = false;

  tspTag spTag = nullptr;
  qint32 iId = -1;
  if (auto tagIt = spProj->m_vspTags.find(sName); spProj->m_vspTags.end() == tagIt)
  {
    spTag = std::make_shared<STag>(sCategory, sName, sDescribtion);

    QWriteLocker locker(&spProj->m_rwLock);
    spTag->m_spParent = spProj;
    spProj->m_vspTags.insert({sName, spTag});
    iId = spProj->m_iId;
    bChanged = true;
  }
  else
  {
    spTag = tagIt->second;
  }

  tspResource spResource = FindResourceInProject(spProj, sResource);
  if (sResource.isEmpty() || nullptr == spResource)
  {
    // nothing to do for now
  }
  else
  {
    QReadLocker locker(&spResource->m_rwLock);
    auto it = spResource->m_vsResourceTags.find(sName);
    if (spResource->m_vsResourceTags.end() == it)
    {
      spResource->m_vsResourceTags.insert(sName);
      spTag->m_vsResourceRefs.insert(sResource);
      bChanged = true;
    }
  }

  if (bChanged)
  {
    emit SignalTagAdded(iId, sResource, sName);
  }

  return sName;
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::ClearTags(tspProject& spProj)
{
  if (!IsInitialized() || nullptr == spProj) { return; }

  QWriteLocker locker(&spProj->m_rwLock);
  qint32 iId = spProj->m_iId;
  while (0 < spProj->m_vspTags.size())
  {
    auto it = spProj->m_vspTags.begin();
    QString sTag = it->first;

    spProj->m_vspTags.erase(it);

    RemoveLingeringTagReferencesFromResources(spProj, it->second);

    locker.unlock();
    emit SignalTagRemoved(iId, QString(), sTag);
    locker.relock();
  }
}

//----------------------------------------------------------------------------------------
//
tspTag CDatabaseManager::FindTagInProject(tspProject& spProj, QString sName)
{
  if (!IsInitialized() || nullptr == spProj) { return nullptr; }

  QReadLocker locker(&spProj->m_rwLock);
  auto it = spProj->m_vspTags.find(sName);
  if (spProj->m_vspTags.end() != it)
  {
    return it->second;
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::RemoveTag(tspProject& spProj, const QString& sName)
{
  if (!IsInitialized() || nullptr == spProj) { return; }

  bool bRemoved = false;
  qint32 iProjId = -1;
  {
    QWriteLocker locker(&spProj->m_rwLock);
    iProjId = spProj->m_iId;

    auto it = spProj->m_vspTags.find(sName);
    if (spProj->m_vspTags.end() != it)
    {
      RemoveLingeringTagReferencesFromResources(spProj, it->second);
      spProj->m_vspTags.erase(it);
      bRemoved = true;
    }
  }

  if (bRemoved)
  {
    emit SignalTagRemoved(iProjId, QString(), sName);
  }
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::RemoveTagFromResource(tspProject& spProj, const QString& sResource, const QString& sName)
{
  if (!IsInitialized() || nullptr == spProj || sResource.isEmpty()) { return; }

  bool bRemoved = true;
  bool bRemovedFromProject = false;
  qint32 iProjId = -1;
  {
    QWriteLocker lockerProj(&spProj->m_rwLock);
    iProjId = spProj->m_iId;

    auto it = spProj->m_vspTags.find(sName);
    if (spProj->m_vspTags.end() != it)
    {
      QWriteLocker locker(&it->second->m_rwLock);
      auto itRes = it->second->m_vsResourceRefs.find(sResource);
      bRemoved &= it->second->m_vsResourceRefs.end() != itRes;
      if (bRemoved)
      {
        it->second->m_vsResourceRefs.erase(itRes);
      }

      if (it->second->m_vsResourceRefs.empty())
      {
        spProj->m_vspTags.erase(it);
        bRemovedFromProject = true;
      }

      auto itResourse = spProj->m_spResourcesMap.find(sResource);
      if (spProj->m_spResourcesMap.end() != itResourse)
      {
        QWriteLocker locker(&itResourse->second->m_rwLock);
        auto itRes = itResourse->second->m_vsResourceTags.find(sName);
        if (itResourse->second->m_vsResourceTags.end() != itRes)
        {
          itResourse->second->m_vsResourceTags.erase(itRes);
        }
      }
    }
  }

  if (bRemoved)
  {
    emit SignalTagRemoved(iProjId, bRemovedFromProject ? QString() : sResource, sName);
  }
}

//----------------------------------------------------------------------------------------
//
QStringList CDatabaseManager::TagCategories(const tspProject& spProj)
{
  if (!IsInitialized()) { return QStringList(); }

  QStringList res;
  QReadLocker locker(&spProj->m_rwLock);
  for (const auto& [_, spTag] : spProj->m_vspTags)
  {
    QReadLocker tagLocker(&spTag->m_rwLock);
    if (!res.contains(spTag->m_sType))
    {
      res << spTag->m_sType;
    }
  }

  return res;
}

//----------------------------------------------------------------------------------------
//
tspKink CDatabaseManager::FindKink(QString sName)
{
  QMutexLocker locker(m_spData.get());
  tspKink kink = nullptr;
  for (auto itCategory = m_spData->m_kinkKategoryMap.begin(); m_spData->m_kinkKategoryMap.end() != itCategory; ++itCategory)
  {
    for (auto it = itCategory->second.begin(); itCategory->second.end() != it; ++it)
    {
      if (it->first == sName)
      {
        kink = it->second;
        goto foundKinkLblBreakOut;
      }
    }
  }
foundKinkLblBreakOut:
  return kink;
}

//----------------------------------------------------------------------------------------
//
tspKink CDatabaseManager::FindKink(QString sCategory, QString sName)
{
  QMutexLocker locker(m_spData.get());
  auto itCategory = m_spData->m_kinkKategoryMap.find(sCategory);
  if (m_spData->m_kinkKategoryMap.end() != itCategory)
  {
    auto itKink = itCategory->second.find(sName);
    if (itCategory->second.end() != itKink)
    {
      return itKink->second;
    }
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
QStringList CDatabaseManager::FindKinks(QString sCategory)
{
  QMutexLocker locker(m_spData.get());
  QStringList vsRet;
  auto itCategory = m_spData->m_kinkKategoryMap.find(sCategory);
  if (m_spData->m_kinkKategoryMap.end() != itCategory)
  {
    for (auto it = itCategory->second.begin(); itCategory->second.end() != it; ++it)
    {
      vsRet << it->first;
    }
  }
  return vsRet;
}

//----------------------------------------------------------------------------------------
//
QStringList CDatabaseManager::KinkCategories()
{
  QMutexLocker locker(m_spData.get());
  QStringList vsRet;
  for (auto it = m_spData->m_kinkKategoryMap.begin(); m_spData->m_kinkKategoryMap.end() != it; ++it)
  {
    vsRet << it->first;
  }
  return vsRet;
}

//----------------------------------------------------------------------------------------
//
QString CDatabaseManager::AddAchievement(
    tspProject& spProj, const QString& sName, const QString& sDescribtion,
    qint32 iType, const QString& sResource, const QVariant& data)
{
  if (!IsInitialized() || nullptr == spProj || sName.isEmpty()) { return QString(); }

  bool bChanged = false;

  tspSaveData spAchievement = nullptr;
  qint32 iId = -1;
  if (auto tagIt = spProj->m_vspAchievements.find(sName); spProj->m_vspAchievements.end() == tagIt)
  {
    spAchievement = std::make_shared<SSaveData>(sName, sDescribtion,
                                                ESaveDataType::_from_integral(iType),
                                                sResource, data);

    QWriteLocker locker(&spProj->m_rwLock);
    spAchievement->m_spParent = spProj;
    spProj->m_vspAchievements.insert({sName, spAchievement});
    iId = spProj->m_iId;
    bChanged = true;
  }
  else
  {
    spAchievement = tagIt->second;
  }

  if (bChanged)
  {
    emit SignalAchievementAdded(iId, sName);
  }

  return sName;
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::ClearAchievement(tspProject& spProj)
{
  if (!IsInitialized() || nullptr == spProj) { return; }

  QWriteLocker locker(&spProj->m_rwLock);
  qint32 iId = spProj->m_iId;
  while (0 < spProj->m_vspAchievements.size())
  {
    auto it = spProj->m_vspAchievements.begin();
    QString sName = it->first;

    spProj->m_vspAchievements.erase(it);

    locker.unlock();
    emit SignalAchievementRemoved(iId, sName);
    locker.relock();
  }
}

//----------------------------------------------------------------------------------------
//
tspSaveData CDatabaseManager::FindAchievementInProject(tspProject& spProj, QString sName)
{
  if (!IsInitialized() || nullptr == spProj) { return nullptr; }

  QReadLocker locker(&spProj->m_rwLock);
  auto it = spProj->m_vspAchievements.find(sName);
  if (spProj->m_vspAchievements.end() != it)
  {
    return it->second;
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::RemoveAchievement(tspProject& spProj, const QString& sName)
{
  if (!IsInitialized() || nullptr == spProj) { return; }

  bool bRemoved = false;
  qint32 iProjId = -1;
  {
    QWriteLocker locker(&spProj->m_rwLock);
    iProjId = spProj->m_iId;

    auto it = spProj->m_vspAchievements.find(sName);
    if (spProj->m_vspAchievements.end() != it)
    {
      spProj->m_vspAchievements.erase(it);
      bRemoved = true;
    }
  }

  if (bRemoved)
  {
    emit SignalAchievementRemoved(iProjId, sName);
  }
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::RenameAchievement(tspProject& spProj, const QString& sName,
                                         const QString& sNewName)
{
  if (!IsInitialized() || nullptr == spProj) { return; }

  tspSaveData spNewAchievement = FindAchievementInProject(spProj, sNewName);
  if (nullptr == spNewAchievement)
  {
    tspSaveData spAchievement = FindAchievementInProject(spProj, sName);

    QWriteLocker locker(&spProj->m_rwLock);
    qint32 iProjId = spProj->m_iId;
    auto it = spProj->m_vspAchievements.find(sName);
    if (it != spProj->m_vspAchievements.end())
    {
      spProj->m_vspAchievements.erase(it);
      {
        QWriteLocker l(&spAchievement->m_rwLock);
        spAchievement->m_sName = sNewName;
      }
      spProj->m_vspAchievements[sNewName] = spAchievement;

      locker.unlock();
      emit SignalAchievementRenamed(iProjId, sName, sNewName);
    }
  }
}

//----------------------------------------------------------------------------------------
//
qint32 CDatabaseManager::FindNewProjectId()
{
  std::set<qint32, std::less<qint32>> ids = ProjectIds();
  return FindNewIdFromSet(ids);
}

//----------------------------------------------------------------------------------------
//
bool CDatabaseManager::IsDbLoaded() const
{
  return m_spDbIo->IsDbLoaded();
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::Initialize()
{
  connect(m_spSettings.get(), &CSettings::contentFolderChanged,
          this, &CDatabaseManager::SlotContentFolderChanged, Qt::QueuedConnection);

  SetInitialized(true);

  SlotContentFolderChanged();
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::Deinitialize()
{
  disconnect(m_spSettings.get(), &CSettings::contentFolderChanged,
          this, &CDatabaseManager::SlotContentFolderChanged);

  ClearProjects();

  SetInitialized(false);
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::SlotContentFolderChanged()
{
  emit SignalReloadStarted();
  ClearProjects();
  m_spDbIo->LoadDatabase();
  emit SignalReloadFinished();
}

//----------------------------------------------------------------------------------------
//
qint32 CDatabaseManager::AddProjectPrivate(const QString& sName,
                                           const QString& sDirNameResolved,
                                           const QString& sProjectPath,
                                           qint32 iNewId,
                                           quint32 iVersion,
                                           bool bBundled, bool bReadOnly,
                                           const tvfnActionsProject& vfnActionsAfterAdding)
{
  QMutexLocker locker(m_spData.get());
  if (0 <= iNewId)
  {
    m_spData->m_vspProjectDatabase.push_back(std::make_shared<SProject>());
    m_spData->m_vspProjectDatabase.back()->m_iId = iNewId;
    m_spData->m_vspProjectDatabase.back()->m_sName = sName;
    m_spData->m_vspProjectDatabase.back()->m_sFolderName = sDirNameResolved;
    m_spData->m_vspProjectDatabase.back()->m_sProjectPath = sProjectPath;
    m_spData->m_vspProjectDatabase.back()->m_iVersion = iVersion;
    m_spData->m_vspProjectDatabase.back()->m_bBundled = bBundled;
    m_spData->m_vspProjectDatabase.back()->m_bReadOnly = bReadOnly;
    m_spData->m_vspProjectDatabase.back()->m_sPlayerLayout = "qrc:/qml/resources/qml/JoipEngine/PlayerDefaultLayout.qml";
    locker.unlock();

    for (auto fn : vfnActionsAfterAdding)
    {
      if (nullptr != fn) { fn(m_spData->m_vspProjectDatabase.back()); }
    }

    emit SignalProjectAdded(iNewId);
  }

  return iNewId;
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
void CDatabaseManager::RemoveLingeringTagReferencesFromResources(tspProject& spProj,
                                                                 tspTag& spTag)
{
  QWriteLocker tagLocker(&spProj->m_rwLock);
  for (const QString& sRes : spTag->m_vsResourceRefs)
  {
    auto itRes = spProj->m_spResourcesMap.find(sRes);
    if (spProj->m_spResourcesMap.end() != itRes)
    {
      QWriteLocker resLocker(&itRes->second->m_rwLock);
      auto itTag = itRes->second->m_vsResourceTags.find(spTag->m_sName);
      if (itRes->second->m_vsResourceTags.end() != itTag)
      {
        itRes->second->m_vsResourceTags.erase(itTag);
      }
    }
  }
}
