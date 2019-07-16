#include "Scene.h"
#include "Project.h"
#include <QMutexLocker>
#include <QScriptEngine>
#include <cassert>

SScene::SScene() = default;
SScene::SScene(const SScene& other) :
  m_spParent(other.m_spParent),
  m_mutex(),
  m_iId(other.m_iId),
  m_sName(other.m_sName),
  m_sScript(other.m_sScript),
  m_vsResourceRefs(other.m_vsResourceRefs)
{}

//----------------------------------------------------------------------------------------
//
CScene::CScene(const std::shared_ptr<SScene>& spScene) :
  QObject(),
  m_spData(spScene)
{
  assert(nullptr != spScene);
  m_spData->m_mutex.lock();
}

CScene::~CScene()
{
  m_spData->m_mutex.unlock();
}

//----------------------------------------------------------------------------------------
//
void CScene::SetId(qint32 iValue)
{
  m_spData->m_iId = iValue;
}

//----------------------------------------------------------------------------------------
//
qint32 CScene::Id()
{
  return m_spData->m_iId;
}

//----------------------------------------------------------------------------------------
//
void CScene::SetName(const QString& sValue)
{
  m_spData->m_sName = sValue;
}

//----------------------------------------------------------------------------------------
//
QString CScene::Name()
{
  return m_spData->m_sName;
}

//----------------------------------------------------------------------------------------
//
void CScene::SetScript(const QString& sValue)
{
  m_spData->m_sScript = sValue;
}

//----------------------------------------------------------------------------------------
//
QString CScene::Script()
{
  return m_spData->m_sScript;
}

//----------------------------------------------------------------------------------------
//
void CScene::AddResource(const QString& sValue)
{
  if (nullptr != m_spData->m_spParent)
  {
    m_spData->m_spParent->m_mutex.lock();
    bool bOk = m_spData->m_spParent->m_resources.find(sValue) != m_spData->m_spParent->m_resources.end();
    m_spData->m_spParent->m_mutex.unlock();
    if (bOk)
    {
      m_spData->m_vsResourceRefs.insert(sValue);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScene::ClearResources()
{
  m_spData->m_vsResourceRefs.clear();
}

//----------------------------------------------------------------------------------------
//
qint32 CScene::NumResources()
{
  return static_cast<qint32>(m_spData->m_vsResourceRefs.size());
}

//----------------------------------------------------------------------------------------
//
void CScene::RemoveResource(const QString& sValue)
{
  auto it = m_spData->m_vsResourceRefs.find(sValue);
  if (it != m_spData->m_vsResourceRefs.end())
  {
    m_spData->m_vsResourceRefs.erase(it);
  }
}

//----------------------------------------------------------------------------------------
//
tspResourceRef CScene::Resource(const QString& sValue)
{
  if (nullptr != m_spData->m_spParent)
  {
    auto it = m_spData->m_vsResourceRefs.find(sValue);
    if (it != m_spData->m_vsResourceRefs.end())
    {
      QMutexLocker locker(&m_spData->m_spParent->m_mutex);
      auto itRef = m_spData->m_spParent->m_resources.find(sValue);
      if (m_spData->m_spParent->m_resources.end() != itRef)
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
