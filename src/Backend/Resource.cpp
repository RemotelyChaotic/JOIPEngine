#include "Resource.h"
#include <QMutexLocker>
#include <QScriptEngine>

CResource::CResource() :
  QObject(),
  m_mutex(),
  m_data(EResourceType::eOther)
{}

CResource::CResource(const CResource& other) :
  QObject(),
  m_mutex(),
  m_data(other.m_data)
{}

CResource::~CResource()
{}

//----------------------------------------------------------------------------------------
//
void CResource::SetPath(const QString& sValue)
{
  QMutexLocker locker(&m_mutex);
  m_data.m_sPath = sValue;
}

//----------------------------------------------------------------------------------------
//
QString CResource::Path()
{
  QMutexLocker locker(&m_mutex);
  return m_data.m_sPath;
}

//----------------------------------------------------------------------------------------
//
void CResource::SetType(qint32 type)
{
  QMutexLocker locker(&m_mutex);
  if (0 <= type && EResourceType::_size() > static_cast<size_t>(type))
  {
    m_data.m_type = EResourceType::_from_index(static_cast<size_t>(type));
  }
}

//----------------------------------------------------------------------------------------
//
qint32 CResource::Type()
{
  QMutexLocker locker(&m_mutex);
  return m_data.m_type;
}

//----------------------------------------------------------------------------------------
//
SResource CResource::Data()
{
  QMutexLocker locker(&m_mutex);
  return m_data;
}

//----------------------------------------------------------------------------------------
//
QScriptValue ResourceToScriptValue(QScriptEngine* pEngine, CResource* const& pIn)
{
  return pEngine->newQObject(pIn);
}

//----------------------------------------------------------------------------------------
//
void ResourceFromScriptValue(const QScriptValue& object, CResource*& pOut)
{
  pOut = qobject_cast<CResource*>(object.toQObject());
}
