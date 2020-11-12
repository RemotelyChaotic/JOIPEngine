#include "Resource.h"
#include "Application.h"
#include "Project.h"

#include <QFileInfo>
#include <QImageReader>
#include <QMutexLocker>
#include <cassert>

//----------------------------------------------------------------------------------------
//
SResource::SResource(EResourceType type) :
  m_spParent(nullptr),
  m_rwLock(QReadWriteLock::Recursive),
  m_sName(),
  m_sPath(),
  m_sSource(),
  m_type(type)
{}

SResource::SResource(const SResource& other) :
  m_spParent(other.m_spParent),
  m_rwLock(QReadWriteLock::Recursive),
  m_sName(other.m_sName),
  m_sPath(other.m_sPath),
  m_sSource(other.m_sSource),
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
    { "sPath", m_sPath.toString(QUrl::None) },
    { "sSource", m_sSource.toString(QUrl::None) },
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
    m_sPath = QUrl(it.value().toString());
  }
  it = json.find("sSource");
  if (it != json.end())
  {
    m_sSource = QUrl(it.value().toString());
  }
  it = json.find("type");
  if (it != json.end())
  {
    // needed for compatibility with older versions
    QString sProject;
    bool bBundled = false;
    if (nullptr != m_spParent)
    {
      // no lock needed here, WARNING not very safe way of doing this
      sProject = m_spParent->m_sName;
      bBundled = m_spParent->m_bBundled;
    }
    locker.unlock();
    const QString sSuffix = QFileInfo(ResourceUrlToAbsolutePath(GetPtr())).suffix();
    locker.relock();

    qint32 iValue = it.value().toInt();
    if (iValue == EResourceType::eOther &&
        DatabaseFormats().contains("*." + sSuffix))
    {
      m_type = EResourceType::eDatabase;
    }
    else if (iValue == EResourceType::eOther &&
        ScriptFormats().contains("*." + sSuffix))
    {
      m_type = EResourceType::eScript;
    }
    else
    {
      m_type = EResourceType::_from_integral(iValue);
    }
  }
}

//----------------------------------------------------------------------------------------
//
CResource::CResource(QJSEngine* pEngine, const std::shared_ptr<SResource>& spResource) :
  QObject(),
  m_spData(spResource),
  m_pEngine(pEngine)
{
  assert(nullptr != spResource);
  assert(nullptr != pEngine);
}

CResource::~CResource()
{
}

//----------------------------------------------------------------------------------------
//
bool CResource::isAnimatedImpl()
{
  QReadLocker locker(&m_spData->m_rwLock);
  switch (m_spData->m_type)
  {
    case EResourceType::eImage:
    {
      if (m_spData->m_sPath.isLocalFile())
      {
        locker.unlock();
        QImageReader reader(ResourceUrlToAbsolutePath(m_spData));
        if (reader.canRead())
        {
          return reader.supportsAnimation();
        }
        else
        {
          return false;
        }
      }
      else
      {
        return false;
      }
    }
    case EResourceType::eMovie: return true;
    default: return false;
  }
}

//----------------------------------------------------------------------------------------
//
bool CResource::isLocalPath()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sPath.isLocalFile();
}

//----------------------------------------------------------------------------------------
//
QString CResource::getName()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sName;
}

//----------------------------------------------------------------------------------------
//
QUrl CResource::getPath()
{
  if (nullptr == m_spData->m_spParent)
  {
    return m_spData->m_sPath;
  }

  const QString sTruePathName = PhysicalProjectName(m_spData->m_spParent);
  QReadLocker projectLocker(&m_spData->m_spParent->m_rwLock);
  bool bBundled = m_spData->m_spParent->m_bBundled;
  projectLocker.unlock();

  QReadLocker locker(&m_spData->m_rwLock);
  if (m_spData->m_sPath.isLocalFile())
  {
    if (!bBundled)
    {
      QUrl urlCopy(m_spData->m_sPath);
      QUrl baseUrl = QUrl::fromLocalFile(CApplication::Instance()->Settings()->ContentFolder() +
        "/" + sTruePathName + "/");
      urlCopy.setScheme(QString());
      QUrl sFullPath = baseUrl.resolved(urlCopy);
      return sFullPath;
    }
    else
    {
      return QUrl("qrc:/" + sTruePathName + "/" + m_spData->m_sName);
    }
  }
  else
  {
    return m_spData->m_sPath;
  }
}

//----------------------------------------------------------------------------------------
//
QUrl CResource::getSource()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sSource;
}

//----------------------------------------------------------------------------------------
//
CResource::ResourceType CResource::getType()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return ResourceType(m_spData->m_type._to_integral());
}

//----------------------------------------------------------------------------------------
//
QJSValue CResource::project()
{
  QReadLocker locker(&m_spData->m_rwLock);
  if (nullptr != m_spData->m_spParent)
  {
    CProject* pProject =
        new CProject(m_pEngine, std::make_shared<SProject>(*m_spData->m_spParent));
    return m_pEngine->newQObject(pProject);
  }
  return QJSValue();
}

//----------------------------------------------------------------------------------------
//
QStringList AudioFormats()
{
  // TODO: check codecs
  return  QStringList() <<
    "*.wav" << "*.dts" <<
    "*.mp3" << "*.mp2" <<
    "*.tta" << "*.tac" <<
    "*.flac" << "*.dts" << "*.aac" <<
    "*.xa ";
}

//----------------------------------------------------------------------------------------
//
QStringList DatabaseFormats()
{
   return QStringList() << "*.json" << "*.xml";
}

//----------------------------------------------------------------------------------------
//
QStringList ImageFormats()
{
  QList<QByteArray> imageFormats = QImageReader::supportedImageFormats();
  QStringList imageFormatsList;
  QString sImageFormats;
  for (QByteArray arr : imageFormats) { imageFormatsList += "*." + QString::fromUtf8(arr); }
  return imageFormatsList;
}

//----------------------------------------------------------------------------------------
//
QStringList OtherFormats()
{
  return QStringList() << "*.json" << "*.proj" << "*.flow" << ".layout";
}

//----------------------------------------------------------------------------------------
//
QString ResourceUrlToAbsolutePath(const tspResource& spResource)
{
  if (nullptr == spResource || nullptr == spResource->m_spParent)
  {
    return QString();
  }

  const QString sTruePathName = PhysicalProjectName(spResource->m_spParent);
  QReadLocker projectLocker(&spResource->m_spParent->m_rwLock);
  bool bBundled = spResource->m_spParent->m_bBundled;
  projectLocker.unlock();

  QReadLocker locker(&spResource->m_rwLock);
  if (spResource->m_sPath.isLocalFile())
  {
    if (!bBundled)
    {
      QUrl urlCopy(spResource->m_sPath);
      QUrl baseUrl = QUrl::fromLocalFile(CApplication::Instance()->Settings()->ContentFolder() +
        "/" + sTruePathName + "/");
      urlCopy.setScheme(QString());
      QUrl sFullPath = baseUrl.resolved(urlCopy);
      return sFullPath.toLocalFile();
    }
    else
    {
      return ":/" + sTruePathName + "/" + spResource->m_sName;
    }
  }
  else
  {
    return QString();
  }
}

//----------------------------------------------------------------------------------------
//
QStringList ScriptFormats()
{
  return QStringList() << "*.js";
}

//----------------------------------------------------------------------------------------
//
QStringList VideoFormats()
{
  // TODO: check codecs
  return QStringList()
      << ".3gp" << "*.asf" << "*.wmv" << "*.mpg" << "*.ts"
         "*.au" << "*.avi" << "*.flv" << "*.mov" << "*.mp4" << "*.ogm" <<
         "*.ogg" << "*.mkv" << "*.mka" <<
         "*.nsc" << "*.nsv" << "*.nut" << "*.a52" <<
         "*.dv" << "*.vid" << "*.ty" << "*.webm";
}
