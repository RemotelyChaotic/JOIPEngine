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
  m_sTitleCard(),
  m_sMap(),
  m_vspScenes(),
  m_spResourcesMap()
{}
SProject::SProject(const SProject& other) :
  m_rwLock(QReadWriteLock::Recursive),
  m_iId(other.m_iId),
  m_iVersion(other.m_iVersion),
  m_sName(other.m_sName),
  m_sTitleCard(other.m_sTitleCard),
  m_sMap(other.m_sMap),
  m_vspScenes(other.m_vspScenes),
  m_spResourcesMap(other.m_spResourcesMap)
{}

SProject::~SProject() {}

//----------------------------------------------------------------------------------------
//
QJsonObject SProject::ToJsonObject()
{
  QWriteLocker locker(&m_rwLock);
  QJsonArray scenes;
  for (auto& spScene : m_vspScenes)
  {
    scenes.push_back(spScene->ToJsonObject());
  }
  QJsonObject resources({});
  for (auto& spResource : m_spResourcesMap)
  {
    resources.insert(spResource.first, spResource.second->ToJsonObject());
  }
  return {
    { "iId", m_iId },
    { "iVersion", m_iVersion },
    { "sName", m_sName },
    { "sTitleCard", m_sTitleCard },
    { "sMap", m_sMap },
    { "vspScenes", scenes },
    { "spResources", resources },
  };
}

//----------------------------------------------------------------------------------------
//
void SProject::FromJsonObject(const QJsonObject& json)
{
  QWriteLocker locker(&m_rwLock);
  auto it = json.find("iId");
  if (it != json.end())
  {
    m_iId = it.value().toInt();
  }
  it = json.find("iVersion");
  if (it != json.end())
  {
    m_iVersion = it.value().toInt();
  }
  it = json.find("sName");
  if (it != json.end())
  {
    m_sName = it.value().toString();
  }
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
  it = json.find("vspScenes");
  if (it != json.end())
  {
    for (QJsonValue val : it.value().toArray())
    {
      m_vspScenes.push_back(std::make_shared<SScene>());
      m_vspScenes.back()->FromJsonObject(val.toObject());
      m_vspScenes.back()->m_spParent = GetPtr();
    }
  }
  it = json.find("spResources");
  if (it != json.end())
  {
    QJsonObject obj = it.value().toObject();
    for (auto objIt = obj.begin(); it != obj.end(); ++it)
    {
      SResource resource;
      resource.FromJsonObject(objIt.value().toObject());
      m_spResourcesMap.insert({objIt.key(), std::make_shared<SResource>(resource)});
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
