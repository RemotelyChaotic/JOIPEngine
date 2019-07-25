#include "Scene.h"
#include "Project.h"
#include "Resource.h"
#include <QJsonArray>
#include <QMutexLocker>
#include <QScriptEngine>
#include <cassert>

SScene::SScene() :
  m_spParent(nullptr),
  m_rwLock(QReadWriteLock::Recursive),
  m_iId(),
  m_sName(),
  m_sScript(),
  m_vsResourceRefs()
{}
SScene::SScene(const SScene& other) :
  m_spParent(other.m_spParent),
  m_rwLock(QReadWriteLock::Recursive),
  m_iId(other.m_iId),
  m_sName(other.m_sName),
  m_sScript(other.m_sScript),
  m_vsResourceRefs(other.m_vsResourceRefs)
{}

SScene::~SScene() {}

//----------------------------------------------------------------------------------------
//
QJsonObject SScene::ToJsonObject()
{
  QWriteLocker locker(&m_rwLock);
  QJsonArray resourceRefs;
  for (QString sRef : m_vsResourceRefs)
  {
    resourceRefs.push_back(sRef);
  }
  return {
    { "iId", m_iId },
    { "sName", m_sName },
    { "sScript", m_sScript },
    { "vsResourceRefs", resourceRefs },
  };
}

//----------------------------------------------------------------------------------------
//
void SScene::FromJsonObject(const QJsonObject& json)
{
  QWriteLocker locker(&m_rwLock);
  auto it = json.find("iId");
  if (it != json.end())
  {
    m_iId = it.value().toInt();
  }
  it = json.find("sName");
  if (it != json.end())
  {
    m_sName = it.value().toString();
  }
  it = json.find("sScript");
  if (it != json.end())
  {
    m_sScript = it.value().toString();
  }
  it = json.find("vsResourceRefs");
  if (it != json.end())
  {
    for (QJsonValue val : it.value().toArray())
    {
      m_vsResourceRefs.insert(val.toString());
    }
  }
}

//----------------------------------------------------------------------------------------
//
CScene::CScene(const std::shared_ptr<SScene>& spScene) :
  QObject(),
  m_spData(spScene)
{
  assert(nullptr != spScene);
}

CScene::~CScene()
{
}

//----------------------------------------------------------------------------------------
//
qint32 CScene::Id()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_iId;
}

//----------------------------------------------------------------------------------------
//
QString CScene::Name()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sName;
}

//----------------------------------------------------------------------------------------
//
QString CScene::Script()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sScript;
}

//----------------------------------------------------------------------------------------
//
qint32 CScene::NumResources()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return static_cast<qint32>(m_spData->m_vsResourceRefs.size());
}

//----------------------------------------------------------------------------------------
//
tspResourceRef CScene::Resource(const QString& sValue)
{
  QReadLocker locker(&m_spData->m_rwLock);
  if (nullptr != m_spData->m_spParent)
  {
    auto it = m_spData->m_vsResourceRefs.find(sValue);
    if (it != m_spData->m_vsResourceRefs.end())
    {
      QReadLocker locker(&m_spData->m_spParent->m_rwLock);
      auto itRef = m_spData->m_spParent->m_spResourcesMap.find(sValue);
      if (m_spData->m_spParent->m_spResourcesMap.end() != itRef)
      {
        locker.unlock();
        return tspResourceRef(new CResource(std::make_shared<SResource>(*itRef->second)));
      }
      return nullptr;
    }
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
tspProjectRef CScene::Project()
{
  QReadLocker locker(&m_spData->m_rwLock);
  if (nullptr != m_spData->m_spParent)
  {
    return tspProjectRef(new CProject(std::make_shared<SProject>(*m_spData->m_spParent)));
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
QScriptValue SceneToScriptValue(QScriptEngine* pEngine, CScene* const& pIn)
{
  return pEngine->newQObject(pIn);
}

//----------------------------------------------------------------------------------------
//
void SceneFromScriptValue(const QScriptValue& object, CScene*& pOut)
{
  pOut = qobject_cast<CScene*>(object.toQObject());
}
