#include "Project.h"
#include <QJsonArray>
#include <QMutexLocker>
#include <cassert>

SProject::SProject() :
  m_rwLock(QReadWriteLock::Recursive),
  m_iId(),
  m_iVersion(),
  m_iTargetVersion(),
  m_sName(),
  m_sOldName(),
  m_sTitleCard(),
  m_sMap(),
  m_sSceneModel(),
  m_bUsesWeb(false),
  m_bNeedsCodecs(false),
  m_vspScenes(),
  m_spResourcesMap()
{}
SProject::SProject(const SProject& other) :
  m_rwLock(QReadWriteLock::Recursive),
  m_iId(other.m_iId),
  m_iVersion(other.m_iVersion),
  m_iTargetVersion(other.m_iTargetVersion),
  m_sName(other.m_sName),
  m_sOldName(other.m_sOldName),
  m_sTitleCard(other.m_sTitleCard),
  m_sMap(other.m_sMap),
  m_sSceneModel(other.m_sSceneModel),
  m_bUsesWeb(other.m_bUsesWeb),
  m_bNeedsCodecs(other.m_bNeedsCodecs),
  m_vspScenes(other.m_vspScenes),
  m_spResourcesMap(other.m_spResourcesMap)
{}

SProject::~SProject() {}

//----------------------------------------------------------------------------------------
//
QJsonObject SProject::ToJsonObject()
{
  QWriteLocker locker(&m_rwLock);

  m_sOldName = QString();

  QJsonArray scenes;
  for (auto& spScene : m_vspScenes)
  {
    scenes.push_back(spScene->ToJsonObject());
  }
  QJsonArray resources;
  for (auto& spResource : m_spResourcesMap)
  {
    resources.push_back(spResource.second->ToJsonObject());
  }
  return {
    { "iVersion", static_cast<qint32>(m_iVersion) },
    { "iTargetVersion", static_cast<qint32>(m_iTargetVersion) },
    { "sName", m_sName },
    { "sTitleCard", m_sTitleCard },
    { "sMap", m_sMap },
    { "sSceneModel", m_sSceneModel },
    { "bUsesWeb", m_bUsesWeb },
    { "bNeedsCodecs", m_bNeedsCodecs },
    { "vspScenes", scenes },
    { "vspResources", resources },
  };
}

//----------------------------------------------------------------------------------------
//
void SProject::FromJsonObject(const QJsonObject& json)
{
  QWriteLocker locker(&m_rwLock);
  auto it = json.find("iVersion");
  if (it != json.end())
  {
    m_iVersion = it.value().toInt();
  }
  it = json.find("iTargetVersion");
  if (it != json.end())
  {
    m_iTargetVersion = it.value().toInt();
  }
  it = json.find("sName");
  if (it != json.end())
  {
    m_sName = it.value().toString();
  }
  m_sOldName = QString();
  it = json.find("sTitleCard");
  if (it != json.end())
  {
    m_sTitleCard = it.value().toString();
  }
  it = json.find("sMap");
  if (it != json.end())
  {
    m_sMap = it.value().toString();
  }
  it = json.find("sSceneModel");
  if (it != json.end())
  {
    m_sSceneModel = it.value().toString();
  }
  it = json.find("bUsesWeb");
  if (it != json.end())
  {
    m_bUsesWeb = it.value().toBool();
  }
  it = json.find("bNeedsCodecs");
  if (it != json.end())
  {
    m_bNeedsCodecs = it.value().toBool();
  }
  it = json.find("vspScenes");
  m_vspScenes.clear();
  if (it != json.end())
  {
    for (QJsonValue val : it.value().toArray())
    {
      std::shared_ptr<SScene> spScene = std::make_shared<SScene>();
      spScene->FromJsonObject(val.toObject());
      spScene->m_spParent = GetPtr();
      m_vspScenes.push_back(spScene);
    }
  }
  it = json.find("vspResources");
  m_spResourcesMap.clear();
  if (it != json.end())
  {
    for (QJsonValue val : it.value().toArray())
    {
      std::shared_ptr<SResource> spResource = std::make_shared<SResource>(EResourceType::eOther);
      spResource->FromJsonObject(val.toObject());
      spResource->m_spParent = GetPtr();
      m_spResourcesMap.insert({spResource->m_sName, spResource});
    }
  }
}

//----------------------------------------------------------------------------------------
//
CProject::CProject(QJSEngine* pEngine, const std::shared_ptr<SProject>& spProject) :
  QObject(),
  m_spData(spProject),
  m_pEngine(pEngine)
{
  assert(nullptr != spProject);
  assert(nullptr != pEngine);
}

CProject::~CProject()
{
}

//----------------------------------------------------------------------------------------
//
qint32 CProject::getId()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_iId;
}

//----------------------------------------------------------------------------------------
//
qint32 CProject::getVersion()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_iVersion;
}

//----------------------------------------------------------------------------------------
//
qint32 CProject::getTargetVersion()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_iTargetVersion;
}

//----------------------------------------------------------------------------------------
//
QString CProject::getName()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sName;
}

//----------------------------------------------------------------------------------------
//
QString CProject::getTitleCard()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sTitleCard;
}

//----------------------------------------------------------------------------------------
//
QString CProject::getMap()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sMap;
}

//----------------------------------------------------------------------------------------
//
bool CProject::isUsingWeb()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_bUsesWeb;
}

//----------------------------------------------------------------------------------------
//
bool CProject::isUsingCodecs()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_bNeedsCodecs;
}

//----------------------------------------------------------------------------------------
//
qint32 CProject::numScenes()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return static_cast<qint32>(m_spData->m_vspScenes.size());
}

//----------------------------------------------------------------------------------------
//
QJSValue CProject::scene(qint32 iIndex)
{
  QReadLocker locker(&m_spData->m_rwLock);
  if (0 <= iIndex && m_spData->m_vspScenes.size() > static_cast<size_t>(iIndex))
  {
    return
      m_pEngine->newQObject(
          new CScene(m_pEngine, std::make_shared<SScene>(*m_spData->m_vspScenes[static_cast<size_t>(iIndex)])));
  }
  return QJSValue();
}

//----------------------------------------------------------------------------------------
//
qint32 CProject::numResources()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return static_cast<qint32>(m_spData->m_spResourcesMap.size());
}

//----------------------------------------------------------------------------------------
//
QJSValue CProject::resource(const QString& sValue)
{
  QReadLocker locker(&m_spData->m_rwLock);
  auto it = m_spData->m_spResourcesMap.find(sValue);
  if (m_spData->m_spResourcesMap.end() != it)
  {
    return
      m_pEngine->newQObject(
          new CResource(m_pEngine, std::make_shared<SResource>(*it->second)));
  }
  return QJSValue();
}

//----------------------------------------------------------------------------------------
//
QString PhysicalProjectName(const tspProject& spProject)
{
  QReadLocker locker(&spProject->m_rwLock);
  QString sName = spProject->m_sName;
  if (!spProject->m_sOldName.isNull())
  {
    sName = spProject->m_sOldName;
  }
  return sName;
}
