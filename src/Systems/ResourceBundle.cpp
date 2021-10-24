#include "ResourceBundle.h"
#include "Project.h"

//----------------------------------------------------------------------------------------
//
SResourceBundle::SResourceBundle() :
  m_rwLock(QReadWriteLock::Recursive),
  m_spParent(nullptr),
  m_sName(),
  m_sPath()
{
}
SResourceBundle::SResourceBundle(const SResourceBundle& other) :
  m_rwLock(QReadWriteLock::Recursive),
  m_spParent(other.m_spParent),
  m_sName(other.m_sName),
  m_sPath(other.m_sPath)
{
}
SResourceBundle::~SResourceBundle()
{

}

//----------------------------------------------------------------------------------------
//
QJsonObject SResourceBundle::ToJsonObject()
{
  QWriteLocker locker(&m_rwLock);
  return {
    { "sName", m_sName },
    { "sPath", m_sPath.toString(QUrl::None) }
  };
}

//----------------------------------------------------------------------------------------
//
void SResourceBundle::FromJsonObject(const QJsonObject& json)
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
    m_sPath = QUrl(it.value().toString());
  }
}
