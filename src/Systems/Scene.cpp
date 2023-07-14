#include "Scene.h"
#include "Project.h"
#include "Resource.h"
#include <QJsonArray>
#include <QMutexLocker>
#include <cassert>

SScene::SScene() :
  SSceneData(),
  m_rwLock(QReadWriteLock::Recursive),
  m_spParent(nullptr),
  m_vsResourceRefs()
{}
SScene::SScene(const SScene& other) :
  SSceneData(other),
  m_rwLock(QReadWriteLock::Recursive),
  m_spParent(other.m_spParent),
  m_vsResourceRefs(other.m_vsResourceRefs)
{}

SScene::~SScene() {}

//----------------------------------------------------------------------------------------
//
QJsonObject SScene::ToJsonObject()
{
  QWriteLocker locker(&m_rwLock);
  QJsonArray resourceRefs;
  for (const QString& sRef : m_vsResourceRefs)
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
    for (const QJsonValue& val : it.value().toArray())
    {
      m_vsResourceRefs.insert(val.toString());
    }
  }
}
