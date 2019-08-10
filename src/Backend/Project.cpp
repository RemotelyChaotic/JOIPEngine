#include "Project.h"
#include <QJsonArray>
#include <QMutexLocker>
#include <QScriptEngine>
#include <cassert>

SProject::SProject() :
  m_rwLock(QReadWriteLock::Recursive),
  m_iId(),
  m_iVersion(),
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
    { "iVersion", m_iVersion },
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
      std::shared_ptr<SResource> spResource = std::make_shared<SResource>();
      spResource->FromJsonObject(val.toObject());
      spResource->m_spParent = GetPtr();
      m_spResourcesMap.insert({spResource->m_sName, spResource});
    }
  }
}

//----------------------------------------------------------------------------------------
//
CProject::CProject(const std::shared_ptr<SProject>& spProject) :
  QObject(),
  m_spData(spProject)
{
  assert(nullptr != spProject);
}

CProject::~CProject()
{
}

//----------------------------------------------------------------------------------------
//
qint32 CProject::Id()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_iId;
}

//----------------------------------------------------------------------------------------
//
qint32 CProject::Version()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_iVersion;
}

//----------------------------------------------------------------------------------------
//
QString CProject::Name()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sName;
}

//----------------------------------------------------------------------------------------
//
QString CProject::TitleCard()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sTitleCard;
}

//----------------------------------------------------------------------------------------
//
QString CProject::Map()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sMap;
}

//----------------------------------------------------------------------------------------
//
bool CProject::IsUsingWeb()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_bUsesWeb;
}

//----------------------------------------------------------------------------------------
//
bool CProject::IsUsingCodecs()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_bNeedsCodecs;
}

//----------------------------------------------------------------------------------------
//
qint32 CProject::NumScenes()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return static_cast<qint32>(m_spData->m_vspScenes.size());
}

//----------------------------------------------------------------------------------------
//
tspSceneRef CProject::Scene(qint32 iIndex)
{
  QReadLocker locker(&m_spData->m_rwLock);
  if (0 <= iIndex && m_spData->m_vspScenes.size() > static_cast<size_t>(iIndex))
  {
    return tspSceneRef(new CScene(std::make_shared<SScene>(*m_spData->m_vspScenes[static_cast<size_t>(iIndex)])));
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
qint32 CProject::NumResources()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return static_cast<qint32>(m_spData->m_spResourcesMap.size());
}

//----------------------------------------------------------------------------------------
//
tspResourceRef CProject::Resource(const QString& sValue)
{
  QReadLocker locker(&m_spData->m_rwLock);
  auto it = m_spData->m_spResourcesMap.find(sValue);
  if (m_spData->m_spResourcesMap.end() != it)
  {
    return tspResourceRef(new CResource(std::make_shared<SResource>(*it->second)));
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
QScriptValue ProjectToScriptValue(QScriptEngine* pEngine, CProject* const& pIn)
{
  return pEngine->newQObject(pIn);
}

//----------------------------------------------------------------------------------------
//
void ProjectFromScriptValue(const QScriptValue& object, CProject*& pOut)
{
  pOut = qobject_cast<CProject*>(object.toQObject());
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
