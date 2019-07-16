#include "Project.h"
#include <QMutexLocker>
#include <QScriptEngine>
#include <cassert>

CProject::CProject(const std::shared_ptr<SProject>& spProject) :
  QObject(),
  m_spData(spProject)
{
  assert(nullptr != spProject);
  m_spData->m_mutex.lock();
}

CProject::~CProject()
{
  m_spData->m_mutex.unlock();
}

//----------------------------------------------------------------------------------------
//
void CProject::SetId(qint32 iValue)
{
  m_spData->m_iId = iValue;
}

//----------------------------------------------------------------------------------------
//
qint32 CProject::Id()
{
  return m_spData->m_iId;
}

//----------------------------------------------------------------------------------------
//
void CProject::SetVersion(qint32 iValue)
{
  m_spData->m_iVersion = iValue;
}

//----------------------------------------------------------------------------------------
//
qint32 CProject::Version()
{
  return m_spData->m_iVersion;
}

//----------------------------------------------------------------------------------------
//
void CProject::SetName(const QString& sValue)
{
  m_spData->m_sName = sValue;
}

//----------------------------------------------------------------------------------------
//
QString CProject::Name()
{
  return m_spData->m_sName;
}

//----------------------------------------------------------------------------------------
//
void CProject::SetTitleCard(const QString& sValue)
{
  m_spData->m_sTitleCard = sValue;
}

//----------------------------------------------------------------------------------------
//
QString CProject::TitleCard()
{
  return m_spData->m_sTitleCard;
}

//----------------------------------------------------------------------------------------
//
void CProject::SetMap(const QString& sValue)
{
  m_spData->m_sMap = sValue;
}

//----------------------------------------------------------------------------------------
//
QString CProject::Map()
{
  return m_spData->m_sMap;
}

//----------------------------------------------------------------------------------------
//
void CProject::AddScene(const tspScene& value)
{
  m_spData->m_vsScenes.push_back(value);
}

//----------------------------------------------------------------------------------------
//
void CProject::ClearScenes()
{
  m_spData->m_vsScenes.clear();
}

//----------------------------------------------------------------------------------------
//
void CProject::InsertScene(qint32 iIndex, const tspScene& value)
{
  if (0 <= iIndex && m_spData->m_vsScenes.size() > static_cast<size_t>(iIndex))
  {
    m_spData->m_vsScenes.insert(m_spData->m_vsScenes.begin() + iIndex, value);
  }
  else if (0 > iIndex)
  {
    m_spData->m_vsScenes.insert(m_spData->m_vsScenes.begin(), value);
  }
  else
  {
    m_spData->m_vsScenes.push_back(value);
  }
}

//----------------------------------------------------------------------------------------
//
qint32 CProject::NumScenes()
{
  return static_cast<qint32>(m_spData->m_vsScenes.size());
}

//----------------------------------------------------------------------------------------
//
void CProject::RemoveScene(qint32 iIndex)
{
  if (0 <= iIndex && m_spData->m_vsScenes.size() > static_cast<size_t>(iIndex))
  {
    m_spData->m_vsScenes.erase(m_spData->m_vsScenes.begin() + iIndex);
  }
}

//----------------------------------------------------------------------------------------
//
tspSceneRef CProject::Scene(qint32 iIndex)
{
  if (0 <= iIndex && m_spData->m_vsScenes.size() > static_cast<size_t>(iIndex))
  {
    return tspSceneRef(new CScene(std::make_shared<SScene>(*m_spData->m_vsScenes[static_cast<size_t>(iIndex)])));
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
void CProject::AddResource(const tspResource& value, const QString& sKey)
{
  m_spData->m_resources.insert({sKey, value});
}

//----------------------------------------------------------------------------------------
//
void CProject::ClearResources()
{
  m_spData->m_resources.clear();
  for (const auto& spScene : m_spData->m_vsScenes)
  {
    spScene->m_mutex.lock();
    spScene->m_vsResourceRefs.clear();
    spScene->m_mutex.unlock();
  }
}

//----------------------------------------------------------------------------------------
//
qint32 CProject::NumResources()
{
  return static_cast<qint32>(m_spData->m_resources.size());
}

//----------------------------------------------------------------------------------------
//
void CProject::RemoveResource(const QString& sValue)
{
  auto it = m_spData->m_resources.find(sValue);
  if (m_spData->m_resources.end() != it)
  {
    for (const auto& spScene : m_spData->m_vsScenes)
    {
      spScene->m_mutex.lock();
      auto itRef = spScene->m_vsResourceRefs.find(sValue);
      if (spScene->m_vsResourceRefs.end() != itRef)
      {
        spScene->m_vsResourceRefs.erase(itRef);
      }
      spScene->m_mutex.unlock();
    }
    m_spData->m_resources.erase(it);
  }
}

//----------------------------------------------------------------------------------------
//
tspResourceRef CProject::Resource(const QString& sValue)
{
  auto it = m_spData->m_resources.find(sValue);
  if (m_spData->m_resources.end() != it)
  {
    return tspResourceRef(new CResource(std::make_shared<SResource>(it->second)));
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
