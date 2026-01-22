#include "ResourceBundle.h"
#include "Project.h"
#include "Resource.h"

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
    { "sPath", joip_resource::MakePathSerialized(static_cast<QString>(m_sPath), m_spParent->m_sFolderName) }
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
    m_sPath = joip_resource::CreatePathFromString(it.value().toString(), m_spParent);
  }
}

//----------------------------------------------------------------------------------------
//
QString PhysicalBundlePath(const tspResourceBundle& spResourceBundle)
{
  QReadLocker bundleLocker(&spResourceBundle->m_rwLock);
  QUrl urlForCall = static_cast<QUrl>(spResourceBundle->m_sPath);
  if (spResourceBundle->m_sPath.IsLocalFile())
  {
    QReadLocker projectLocker(&spResourceBundle->m_spParent->m_rwLock);
    QString sPathSer = joip_resource::MakePathSerialized(static_cast<QString>(spResourceBundle->m_sPath),
                                                         spResourceBundle->m_spParent->m_sFolderName);
    urlForCall = QUrl(sPathSer);
  }

  QReadLocker projectLocker(&spResourceBundle->m_spParent->m_rwLock);
  const QString sTrueProjectName = spResourceBundle->m_spParent->m_sName;
  const QString sProjectFolder = spResourceBundle->m_spParent->m_sFolderName;
  bool bBundled = spResourceBundle->m_spParent->m_bBundled;
  projectLocker.unlock();

  if (spResourceBundle->m_sPath.IsLocalFile())
  {
    if (!bBundled)
    {
      QUrl urlCopy(urlForCall);
      urlCopy.setScheme(QString());
      QString sBasePath = PhysicalProjectPath(spResourceBundle->m_spParent);
      return sBasePath + "/" + QUrl().resolved(urlCopy).toString();
    }
    else
    {
      return ":/" + sTrueProjectName + "/" + spResourceBundle->m_sName;
    }
  }
  return urlForCall.toString(QUrl::None);
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
  if (spResourceBundle->m_sPath.IsLocalFile())
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
