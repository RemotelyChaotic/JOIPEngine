#include "Project.h"
#include <QMutexLocker>
#include <QScriptEngine>

CProject::CProject() :
  QObject(),
  m_mutex(),
  m_data()
{}

CProject::CProject(const CProject& other) :
  QObject(),
  m_mutex(),
  m_data(other.m_data)
{}

CProject::~CProject()
{}

//----------------------------------------------------------------------------------------
//
void CProject::SetId(qint32 iValue)
{
  QMutexLocker locker(&m_mutex);
  m_data.m_iId = iValue;
}

//----------------------------------------------------------------------------------------
//
qint32 CProject::Id()
{
  QMutexLocker locker(&m_mutex);
  return m_data.m_iId;
}

//----------------------------------------------------------------------------------------
//
void CProject::SetVersion(qint32 iValue)
{
  QMutexLocker locker(&m_mutex);
  m_data.m_iVersion = iValue;
}

//----------------------------------------------------------------------------------------
//
qint32 CProject::Version()
{
  QMutexLocker locker(&m_mutex);
  return m_data.m_iVersion;
}

//----------------------------------------------------------------------------------------
//
void CProject::SetName(const QString& sValue)
{
  QMutexLocker locker(&m_mutex);
  m_data.m_sName = sValue;
}

//----------------------------------------------------------------------------------------
//
QString CProject::Name()
{
  QMutexLocker locker(&m_mutex);
  return m_data.m_sName;
}

//----------------------------------------------------------------------------------------
//
void CProject::SetTitleCard(const QString& sValue)
{
  QMutexLocker locker(&m_mutex);
  m_data.m_sTitleCard = sValue;
}

//----------------------------------------------------------------------------------------
//
QString CProject::TitleCard()
{
  QMutexLocker locker(&m_mutex);
  return m_data.m_sTitleCard;
}

//----------------------------------------------------------------------------------------
//
void CProject::SetMap(const QString& sValue)
{
  QMutexLocker locker(&m_mutex);
  m_data.m_sMap = sValue;
}

//----------------------------------------------------------------------------------------
//
QString CProject::Map()
{
  QMutexLocker locker(&m_mutex);
  return m_data.m_sMap;
}

//----------------------------------------------------------------------------------------
//
void CProject::AddScene(const tspScene& sValue)
{
  QMutexLocker locker(&m_mutex);
  m_data.m_vsScenes.push_back(sValue);
}

//----------------------------------------------------------------------------------------
//
void CProject::ClearScenes()
{
  QMutexLocker locker(&m_mutex);
  m_data.m_vsScenes.clear();
}

//----------------------------------------------------------------------------------------
//
void CProject::InsertScene(qint32 iIndex, const tspScene& sValue)
{
  QMutexLocker locker(&m_mutex);
  if (0 <= iIndex && m_data.m_vsScenes.size() > static_cast<size_t>(iIndex))
  {
    m_data.m_vsScenes.insert(m_data.m_vsScenes.begin() + iIndex, sValue);
  }
  else if (0 > iIndex)
  {
    m_data.m_vsScenes.insert(m_data.m_vsScenes.begin(), sValue);
  }
  else
  {
    m_data.m_vsScenes.push_back(sValue);
  }
}

//----------------------------------------------------------------------------------------
//
qint32 CProject::NumScenes()
{
  QMutexLocker locker(&m_mutex);
  return static_cast<qint32>(m_data.m_vsScenes.size());
}

//----------------------------------------------------------------------------------------
//
void CProject::RemoveScene(qint32 iIndex)
{
  QMutexLocker locker(&m_mutex);
  if (0 <= iIndex && m_data.m_vsScenes.size() > static_cast<size_t>(iIndex))
  {
    m_data.m_vsScenes.erase(m_data.m_vsScenes.begin() + iIndex);
  }
}

//----------------------------------------------------------------------------------------
//
tspScene CProject::Scene(qint32 iIndex)
{
  QMutexLocker locker(&m_mutex);
  if (0 <= iIndex && m_data.m_vsScenes.size() > static_cast<size_t>(iIndex))
  {
    return m_data.m_vsScenes[static_cast<size_t>(iIndex)];
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
SProject CProject::Data()
{
  QMutexLocker locker(&m_mutex);
  return m_data;
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
