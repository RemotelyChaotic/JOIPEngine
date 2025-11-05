#include "ResourceBundle.h"
#include "Project.h"

#include "Systems/PhysFs/PhysFsFileEngine.h"

//----------------------------------------------------------------------------------------
//
SResourceBundle::SResourceBundle() :
  m_rwLock(QReadWriteLock::Recursive),
  m_spParent(nullptr),
  m_sName(),
  m_sPath(),
  m_bLoaded(false)
{
}
SResourceBundle::SResourceBundle(const SResourceBundle& other) :
  std::enable_shared_from_this<SResourceBundle>(other),
  m_rwLock(QReadWriteLock::Recursive),
  m_spParent(other.m_spParent),
  m_sName(other.m_sName),
  m_sPath(other.m_sPath),
  m_bLoaded(other.m_bLoaded)
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

//----------------------------------------------------------------------------------------
//
QString ResourceBundleUrlToAbsolutePath(const tspResourceBundle& spResourceBundle)
{
  if (nullptr == spResourceBundle || nullptr == spResourceBundle->m_spParent)
  {
    return QString();
  }

  QReadLocker projectLocker(&spResourceBundle->m_spParent->m_rwLock);
  const QString sTrueProjectName = spResourceBundle->m_spParent->m_sName;
  bool bBundled = spResourceBundle->m_spParent->m_bBundled;
  projectLocker.unlock();

  QReadLocker locker(&spResourceBundle->m_rwLock);
  if (IsLocalFile(spResourceBundle->m_sPath))
  {
    if (!bBundled)
    {
      QUrl urlCopy(spResourceBundle->m_sPath);
      urlCopy.setScheme(QString());
      QString sBasePath = CPhysFsFileEngineHandler::c_sScheme;
      return sBasePath + QUrl().resolved(urlCopy).toString();
    }
    else
    {
      return "qrc:/" + sTrueProjectName + "/" + spResourceBundle->m_sName;
    }
  }
  else
  {
    return QString();
  }
}
