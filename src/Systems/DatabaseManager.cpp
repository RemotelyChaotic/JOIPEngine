#include "DatabaseManager.h"
#include "Application.h"
#include "Project.h"
#include "Resource.h"
#include "Scene.h"
#include "Settings.h"

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QJsonDocument>
#include <QResource>

namespace  {
  const char c_sProjectBundleFileEnding[] = ".proj";
}

//----------------------------------------------------------------------------------------
//
CDatabaseManager::CDatabaseManager() :
  CSystemBase(),
  m_spSettings(CApplication::Instance()->Settings()),
  m_bLoadedDb(0),
  m_vspProjectDatabase()
{

}

CDatabaseManager::~CDatabaseManager()
{
}

//----------------------------------------------------------------------------------------
//
bool CDatabaseManager::LoadProject(tspProject& spProject)
{
  if (nullptr == spProject) { return false; }

  const QString sProjectName = PhysicalProjectName(spProject);
  QWriteLocker locker(&spProject->m_rwLock);
  if (spProject->m_bBundled && !spProject->m_bLoaded)
  {
    spProject->m_bLoaded =
        QResource::registerResource(CApplication::Instance()->Settings()->ContentFolder() + "/" +
                                    sProjectName + c_sProjectBundleFileEnding,
                                    "/" + spProject->m_sName);
    return spProject->m_bLoaded;
  }
  else
  {
    return true;
  }
}

//----------------------------------------------------------------------------------------
//
bool CDatabaseManager::UnloadProject(tspProject& spProject)
{
  if (nullptr == spProject) { return false; }

  const QString sProjectName = PhysicalProjectName(spProject);
  QWriteLocker locker(&spProject->m_rwLock);
  if (spProject->m_bBundled && spProject->m_bLoaded)
  {
    spProject->m_bLoaded =
        !QResource::unregisterResource(CApplication::Instance()->Settings()->ContentFolder() + "/" +
                                       sProjectName + c_sProjectBundleFileEnding,
                                       "/" + spProject->m_sName);
    return !spProject->m_bLoaded;
  }
  else
  {
    return true;
  }
}

//----------------------------------------------------------------------------------------
//
qint32 CDatabaseManager::AddProject(const QString& sDirName, quint32 iVersion, bool bBundled,
                                    const tvfnActionsProject& vfnActionsAfterAdding)
{
  if (!IsInitialized()) { return -1; }

  qint32 iNewId = FindNewProjectId();

  QString sName = sDirName;
  QString sError;
  if (!ProjectNameCheck(sDirName, &sError))
  {
    sName = ToValidProjectName(sDirName);
  }
  else
  {
    sName = sDirName;
  }


  QMutexLocker locker(&m_dbMutex);
  if (0 <= iNewId)
  {
    m_vspProjectDatabase.push_back(std::make_shared<SProject>());
    m_vspProjectDatabase.back()->m_iId = iNewId;
    m_vspProjectDatabase.back()->m_sName = sName;
    m_vspProjectDatabase.back()->m_sFolderName = sDirName;
    m_vspProjectDatabase.back()->m_iVersion = iVersion;
    m_vspProjectDatabase.back()->m_bBundled = bBundled;

    locker.unlock();

    for (auto fn : vfnActionsAfterAdding)
    {
      if (nullptr != fn) { fn(m_vspProjectDatabase.back()); }
    }

    emit SignalProjectAdded(iNewId);
  }

  return iNewId;
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::ClearProjects()
{
  if (!IsInitialized()) { return; }

  QMutexLocker locker(&m_dbMutex);
  while (0 < m_vspProjectDatabase.size())
  {
    auto it = m_vspProjectDatabase.begin();
    QReadLocker projLocker(&(*it)->m_rwLock);
    qint32 iId = (*it)->m_iId;
    QString sName = (*it)->m_sName;
    projLocker.unlock();

    UnloadProject(*it);

    m_vspProjectDatabase.erase(it);

    locker.unlock();
    emit SignalProjectRemoved(iId);
    locker.relock();
  }

  m_bLoadedDb = 0;
}

//----------------------------------------------------------------------------------------
//
bool CDatabaseManager::DeserializeProject(qint32 iId)
{
  if (!IsInitialized()) { return false; }

  tspProject spProject = FindProject(iId);
  if (nullptr != spProject)
  {
    return DeserializeProjectPrivate(spProject);
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
    return DeserializeProjectPrivate(spProject);
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
tspProject CDatabaseManager::FindProject(qint32 iId)
{
  if (!IsInitialized()) { return nullptr; }

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
  if (!IsInitialized()) { return nullptr; }

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
std::set<qint32, std::less<qint32>> CDatabaseManager::ProjectIds()
{
  if (!IsInitialized()) { return std::set<qint32>(); }

  QMutexLocker locker(&m_dbMutex);
  std::set<qint32, std::less<qint32>> ids;
  for (tspProject& spProject : m_vspProjectDatabase)
  {
    QReadLocker projLocker(&spProject->m_rwLock);
    ids.insert(spProject->m_iId);
  }
  return ids;
}

//----------------------------------------------------------------------------------------
//
void CDatabaseManager::RemoveProject(qint32 iId)
{
  if (!IsInitialized()) { return; }

  QMutexLocker locker(&m_dbMutex);
  for (auto it = m_vspProjectDatabase.begin(); m_vspProjectDatabase.end() != it; ++it)
  {
    QReadLocker projLocker(&(*it)->m_rwLock);
    qint32 iFoundId = (*it)->m_iId;
    projLocker.unlock();
    if (iFoundId == iId)
    {
      m_vspProjectDatabase.erase(it);

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

  QMutexLocker locker(&m_dbMutex);
  for (auto it = m_vspProjectDatabase.begin(); m_vspProjectDatabase.end() != it; ++it)
  {
    QReadLocker projLocker(&(*it)->m_rwLock);
    qint32 iFoundId = (*it)->m_iId;
    projLocker.unlock();
    if ((*it)->m_sName == sName)
    {
      m_vspProjectDatabase.erase(it);

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
    spProject->m_rwLock.lockForWrite();
    spProject->m_sName = sNewName;
    spProject->m_rwLock.unlock();

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
    spProject->m_rwLock.lockForWrite();
    qint32 iId = spProject->m_iId;
    spProject->m_sName = sNewName;
    spProject->m_rwLock.unlock();

    emit SignalProjectRenamed(iId);
  }
}

//----------------------------------------------------------------------------------------
//
bool CDatabaseManager::SerializeProject(qint32 iId)
{
  if (!IsInitialized()) { return false; }

  tspProject spProject = FindProject(iId);
  if (nullptr != spProject)
  {
    return SerializeProjectPrivate(spProject);
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
bool CDatabaseManager::SerializeProject(const QString& sName)
{
  if (!IsInitialized()) { return false; }

  tspProject spProject = FindProject(sName);
  if (nullptr != spProject)
  {
    return SerializeProjectPrivate(spProject);
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
  if (!sPath.isLocalFile())
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
void CDatabaseManager::RemoveResource(tspProject& spProj, const QString& sName)
{
  if (!IsInitialized() || nullptr == spProj) { return; }

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
      if (spScene->m_sScript == sName)
      {
        spScene->m_sScript = QString();
      }
    }

    locker.unlock();
    emit SignalResourceRemoved(spProj->m_iId, sName);
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

      spProj->m_spResourcesMap.erase(it);
      spResource->m_rwLock.lockForWrite();
      spResource->m_sName = sNewName;
      spResource->m_rwLock.unlock();
      spProj->m_spResourcesMap.insert({sNewName, spResource});

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

      locker.unlock();
      emit SignalResourceRenamed(iProjId, sName, sNewName);
    }
  }
}

//----------------------------------------------------------------------------------------
//
tspKink CDatabaseManager::FindKink(QString sName)
{
  QMutexLocker locker(&m_dbMutex);
  tspKink kink = nullptr;
  for (auto itCategory = m_kinkKategoryMap.begin(); m_kinkKategoryMap.end() != itCategory; ++itCategory)
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
  QMutexLocker locker(&m_dbMutex);
  auto itCategory = m_kinkKategoryMap.find(sCategory);
  if (m_kinkKategoryMap.end() != itCategory)
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
  QMutexLocker locker(&m_dbMutex);
  QStringList vsRet;
  auto itCategory = m_kinkKategoryMap.find(sCategory);
  if (m_kinkKategoryMap.end() != itCategory)
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
  QMutexLocker locker(&m_dbMutex);
  QStringList vsRet;
  for (auto it = m_kinkKategoryMap.begin(); m_kinkKategoryMap.end() != it; ++it)
  {
    vsRet << it->first;
  }
  return vsRet;
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
  LoadDatabase();
  emit SignalReloadFinished();
}

//----------------------------------------------------------------------------------------
//
bool CDatabaseManager::DeserializeProjectPrivate(tspProject& spProject)
{
  spProject->m_rwLock.lockForRead();
  const QString sName = spProject->m_sName;
  bool bBundled = spProject->m_bBundled;
  spProject->m_rwLock.unlock();

  bool bOk = true;
  QDir sContentFolder(m_spSettings->ContentFolder());
  if (!bBundled)
  {
    if (!QFileInfo(m_spSettings->ContentFolder() + QDir::separator() + sName).exists())
    {
      qWarning() << "Could not find project folder: " + m_spSettings->ContentFolder() + QDir::separator() + sName;
    }
  }
  else
  {
    if (!QFileInfo(m_spSettings->ContentFolder() + QDir::separator() + sName + c_sProjectBundleFileEnding).exists())
    {
      qWarning() << "Could not find project bundle file: " + m_spSettings->ContentFolder() + QDir::separator() + sName + c_sProjectBundleFileEnding;
    }
  }

  bOk &= LoadProject(spProject);

  if (bOk)
  {
    QString jsonFile;
    if (!bBundled)
    {
      jsonFile = m_spSettings->ContentFolder() + QDir::separator() + sName +
          QDir::separator() + joip_resource::c_sProjectFileName;
    }
    else
    {
      jsonFile = ":/" + sName + "/" + joip_resource::c_sProjectFileName;
    }

    QFileInfo jsonInfo(jsonFile);
    if (jsonInfo.exists())
    {
      QFile jsonFile(jsonInfo.absoluteFilePath());
      if (jsonFile.open(QIODevice::ReadOnly))
      {
        QJsonParseError err;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonFile.readAll(), &err);
        if (err.error == QJsonParseError::NoError)
        {
          spProject->FromJsonObject(jsonDocument.object());

          spProject->m_rwLock.lockForRead();
          qint32 iOldId = spProject->m_iId;
          spProject->m_rwLock.unlock();
          qint32 iNewId = (-1 == iOldId) ? FindNewProjectId() : -1;
          spProject->m_rwLock.lockForWrite();
          spProject->m_iId = (-1 == iOldId) ? iNewId : iOldId;
          spProject->m_rwLock.unlock();
          bOk = true;
        }
        else
        {
          qWarning() << err.errorString();
          bOk = false;
        }
      }
      else
      {
        qWarning() << "Could not read project file: " + jsonInfo.absoluteFilePath();
        bOk = false;
      }
    }
    else
    {
      qWarning() << "Could find project file: " + jsonInfo.absoluteFilePath();
      bOk = false;
    }
  }

  return bOk & UnloadProject(spProject);
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
  std::set<qint32, std::less<qint32>> ids = ProjectIds();
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
  // load projects
  QString sPath = m_spSettings->ContentFolder();
  QDirIterator it(sPath, QDir::Dirs | QDir::NoDotAndDotDot);
  while (it.hasNext())
  {
    QString sDirName = it.next();
    qint32 index = sDirName.lastIndexOf("/");
    sDirName = sDirName.right(sDirName.length() - index - 1);
    AddProject(sDirName);
  }

  QDirIterator itBundle(sPath, QStringList() << QString("*") + c_sProjectBundleFileEnding,
                        QDir::Files | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
  while (itBundle.hasNext())
  {
    QString sFileName = QFileInfo(itBundle.next()).baseName();
    AddProject(sFileName, 1, true);
  }

  // store projects
  QMutexLocker locker(&m_dbMutex);
  for (tspProject spProject : m_vspProjectDatabase)
  {
    DeserializeProjectPrivate(spProject);
  }

  // load kinks
  QFile kinkData(":/resources/data/Kink_Information_Data.csv");
  if (kinkData.exists() && kinkData.open(QIODevice::ReadOnly))
  {
    QTextStream stream(&kinkData);
    stream.setCodec("UTF-8");

    QString sLine;
    while (stream.readLineInto(&sLine))
    {
      QStringList vsLineData = sLine.split(";");
      if (vsLineData.size() == 3)
      {
        tKinks& kinks = m_kinkKategoryMap[vsLineData[0]];
        tspKink spKink = std::make_shared<SKink>();
        spKink->m_sType = vsLineData[0];
        spKink->m_sName = vsLineData[1];
        spKink->m_sDescribtion = vsLineData[2];
        kinks.insert({vsLineData[1], spKink});
      }
    }
  }

  m_bLoadedDb = 1;
}

//----------------------------------------------------------------------------------------
//
bool CDatabaseManager::SerializeProjectPrivate(tspProject& spProject)
{
  spProject->m_rwLock.lockForRead();
  const QString sName = spProject->m_sName;
  const QString sFolderName = spProject->m_sFolderName;
  bool bBundled = spProject->m_bBundled;
  spProject->m_rwLock.unlock();

  // cannot serialize bundled projects, since these are read only
  if (bBundled) { return true; }

  bool bOk = true;
  QDir sContentFolder(m_spSettings->ContentFolder());

  // first rename old folder
  if (sFolderName != sName)
  {
    if (!sFolderName.isEmpty())
    {
      if (QFileInfo(m_spSettings->ContentFolder() + QDir::separator() + sFolderName).exists())
      {
        bOk = sContentFolder.rename(sFolderName, sName);
      }
      else
      {
        bOk = false;
        qWarning() << "Could not rename folder: " + m_spSettings->ContentFolder() + QDir::separator() + sFolderName;
      }
    }
  }

  // if new doesn't exist -> create
  if (bOk && !QFileInfo(m_spSettings->ContentFolder() + QDir::separator() + sName).exists())
  {
    bOk = sContentFolder.mkdir(sName);
    if (!bOk)
    {
      qWarning() << "Could not create folder: " + m_spSettings->ContentFolder() + QDir::separator() + sName;
    }
  }

  if (bOk)
  {
    spProject->m_rwLock.lockForWrite();
    spProject->m_sFolderName = sName;
    spProject->m_rwLock.unlock();
  }

  QJsonDocument document(spProject->ToJsonObject());

  if (bOk)
  {
    QFileInfo jsonInfo(m_spSettings->ContentFolder() + QDir::separator() + sName +
                       QDir::separator() + joip_resource::c_sProjectFileName);
    QFile jsonFile(jsonInfo.absoluteFilePath());
    if (jsonFile.open(QIODevice::ReadWrite | QIODevice::Truncate))
    {
      jsonFile.write(document.toJson(QJsonDocument::Indented));
      bOk = true;
    }
    else
    {
      qWarning() << "Could not wirte project file: " + jsonInfo.absoluteFilePath();
      bOk =  false;
    }
  }

  return bOk;
}
