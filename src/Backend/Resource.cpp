#include "Resource.h"
#include <QMutexLocker>
#include <QScriptEngine>
#include <cassert>

CResource::CResource(const std::shared_ptr<SResource>& spResource) :
  QObject(),
  m_spData(spResource)
{
  assert(nullptr != spResource);
  m_spData->m_mutex.lock();
}

CResource::~CResource()
{
  m_spData->m_mutex.unlock();
}

//----------------------------------------------------------------------------------------
//
void CResource::SetPath(const QString& sValue)
{
  m_spData->m_sPath = sValue;
}

//----------------------------------------------------------------------------------------
//
QString CResource::Path()
{
  return m_spData->m_sPath;
}

//----------------------------------------------------------------------------------------
//
void CResource::SetType(qint32 type)
{
  if (0 <= type && EResourceType::_size() > static_cast<size_t>(type))
  {
    m_spData->m_type = EResourceType::_from_index(static_cast<size_t>(type));
  }
}

//----------------------------------------------------------------------------------------
//
qint32 CResource::Type()
{
  return m_spData->m_type;
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
