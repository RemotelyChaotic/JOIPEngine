#include "Scene.h"
#include "Project.h"
#include "Resource.h"
#include <QJsonArray>
#include <QMutexLocker>
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
  m_vsResourceRefs.clear();
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
CScene::CScene(QJSEngine* pEngine, const std::shared_ptr<SScene>& spScene) :
  QObject(),
  m_spData(spScene),
  m_pEngine(pEngine)
{
  assert(nullptr != spScene);
  assert(nullptr != pEngine);
}

CScene::~CScene()
{
}

//----------------------------------------------------------------------------------------
//
qint32 CScene::getId()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_iId;
}

//----------------------------------------------------------------------------------------
//
QString CScene::getName()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sName;
}

//----------------------------------------------------------------------------------------
//
QString CScene::getScript()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sScript;
}

//----------------------------------------------------------------------------------------
//
qint32 CScene::numResources()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return static_cast<qint32>(m_spData->m_vsResourceRefs.size());
}

//----------------------------------------------------------------------------------------
//
QStringList CScene::resources()
{
  QReadLocker locker(&m_spData->m_rwLock);
  QStringList ret;
  for (auto it = m_spData->m_vsResourceRefs.begin(); m_spData->m_vsResourceRefs.end() != it; ++it)
  {
    ret << *it;
  }
  return ret;
}

//----------------------------------------------------------------------------------------
//
QJSValue CScene::resource(const QString& sValue)
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
        return
          m_pEngine->newQObject(
              new CResource(m_pEngine, std::make_shared<SResource>(*itRef->second)));
      }
      return QJSValue();
    }
  }
  return QJSValue();
}

//----------------------------------------------------------------------------------------
//
QJSValue CScene::project()
{
  QReadLocker locker(&m_spData->m_rwLock);
  if (nullptr != m_spData->m_spParent)
  {
    return
      m_pEngine->newQObject(
          new CProject(m_pEngine, std::make_shared<SProject>(*m_spData->m_spParent)));
  }
  return QJSValue();
}