#include "Scene.h"
#include <QMutexLocker>
#include <QScriptEngine>

CScene::CScene() :
  QObject(),
  m_mutex(),
  m_data()
{}

CScene::CScene(const CScene& other) :
  QObject(),
  m_mutex(),
  m_data(other.m_data)
{}

CScene::~CScene()
{}

//----------------------------------------------------------------------------------------
//
void CScene::SetId(qint32 iValue)
{
  QMutexLocker locker(&m_mutex);
  m_data.m_iId = iValue;
}

//----------------------------------------------------------------------------------------
//
qint32 CScene::Id()
{
  QMutexLocker locker(&m_mutex);
  return m_data.m_iId;
}

//----------------------------------------------------------------------------------------
//
void CScene::SetName(const QString& sValue)
{
  QMutexLocker locker(&m_mutex);
  m_data.m_sName = sValue;
}

//----------------------------------------------------------------------------------------
//
QString CScene::Name()
{
  QMutexLocker locker(&m_mutex);
  return m_data.m_sName;
}

//----------------------------------------------------------------------------------------
//
void CScene::SetScript(const QString& sValue)
{
  QMutexLocker locker(&m_mutex);
  m_data.m_sScript = sValue;
}

//----------------------------------------------------------------------------------------
//
QString CScene::Script()
{
  QMutexLocker locker(&m_mutex);
  return m_data.m_sScript;
}

//----------------------------------------------------------------------------------------
//
void CScene::AddResource(const tspResource& spValue)
{
  QMutexLocker locker(&m_mutex);
  const QString sPath = spValue->Path();
  if (m_data.m_resources.find(sPath) != m_data.m_resources.end())
  {
    m_data.m_resources.insert({sPath, spValue});
  }
}

//----------------------------------------------------------------------------------------
//
void CScene::ClearResources()
{
  QMutexLocker locker(&m_mutex);
  m_data.m_resources.clear();
}

//----------------------------------------------------------------------------------------
//
qint32 CScene::NumResources()
{
  QMutexLocker locker(&m_mutex);
  return static_cast<qint32>(m_data.m_resources.size());
}

//----------------------------------------------------------------------------------------
//
void CScene::RemoveResource(const QString& sValue)
{
  QMutexLocker locker(&m_mutex);
  auto it = m_data.m_resources.find(sValue);
  if (it != m_data.m_resources.end())
  {
    m_data.m_resources.erase(it);
  }
}

//----------------------------------------------------------------------------------------
//
tspResource CScene::Resource(const QString& sValue)
{
  QMutexLocker locker(&m_mutex);
  auto it = m_data.m_resources.find(sValue);
  if (it != m_data.m_resources.end())
  {
    return it->second;
  }
  else
  {
    return nullptr;
  }
}

//----------------------------------------------------------------------------------------
//
SScene CScene::Data()
{
  QMutexLocker locker(&m_mutex);
  return m_data;
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
