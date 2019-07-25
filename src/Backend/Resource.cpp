#include "Resource.h"
#include <QMutexLocker>
#include <QScriptEngine>
#include <cassert>

SResource::SResource(EResourceType type) :
  m_rwLock(QReadWriteLock::Recursive),
  m_sPath(),
  m_type(type)
{}

SResource::SResource(const SResource& other) :
  m_rwLock(QReadWriteLock::Recursive),
  m_sPath(other.m_sPath),
  m_type(other.m_type)
{}

SResource::~SResource() {}

//----------------------------------------------------------------------------------------
//
QJsonObject SResource::ToJsonObject()
{
  QWriteLocker locker(&m_rwLock);
  return {
    { "sName", m_sName },
    { "sPath", m_sPath },
    { "type", m_type._value },
  };
}

//----------------------------------------------------------------------------------------
//
void SResource::FromJsonObject(const QJsonObject& json)
{
  QWriteLocker locker(&m_rwLock);
  auto it = json.find("sName");
  if (it != json.end())
  {
    m_sName = it.value().toString();
  }
  it = json.find("sPath");
  if (it != json.end())
  {
    m_sPath = it.value().toString();
  }
  it = json.find("type");
  if (it != json.end())
  {
    m_type._from_integral(it.value().toInt());
  }
}

//----------------------------------------------------------------------------------------
//
CResource::CResource(const std::shared_ptr<SResource>& spResource) :
  QObject(),
  m_spData(spResource)
{
  assert(nullptr != spResource);
}

CResource::~CResource()
{
}

//----------------------------------------------------------------------------------------
//
QString CResource::Name()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sName;
}

//----------------------------------------------------------------------------------------
//
QString CResource::Path()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sPath;
}

//----------------------------------------------------------------------------------------
//
qint32 CResource::Type()
{
  QReadLocker locker(&m_spData->m_rwLock);
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
