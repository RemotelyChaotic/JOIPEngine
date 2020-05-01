#include "Resource.h"
#include "Application.h"
#include "Project.h"

#include <QImageReader>
#include <QMutexLocker>
#include <cassert>

SResource::SResource(EResourceType type) :
  m_spParent(nullptr),
  m_rwLock(QReadWriteLock::Recursive),
  m_sName(),
  m_sPath(),
  m_type(type)
{}

SResource::SResource(const SResource& other) :
  m_spParent(other.m_spParent),
  m_rwLock(QReadWriteLock::Recursive),
  m_sName(other.m_sName),
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
    { "sPath", m_sPath.toString(QUrl::None) },
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
  it = json.find("type");
  if (it != json.end())
  {
    qint32 iValue = it.value().toInt();
    m_type = EResourceType::_from_integral(iValue);
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
      QReadLocker projLocker(&m_spData->m_spParent->m_rwLock);
      const QString sProjectName = m_spData->m_spParent->m_sFolderName;
      projLocker.unlock();

      if (m_spData->m_sPath.isLocalFile())
      {
        QImageReader reader(ResourceUrlToAbsolutePath(m_spData->m_sPath, sProjectName));
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
  QReadLocker projLocker(&m_spData->m_spParent->m_rwLock);
  const QString sProjectName = m_spData->m_spParent->m_sFolderName;
  projLocker.unlock();

  QReadLocker locker(&m_spData->m_rwLock);
  if (m_spData->m_sPath.isLocalFile())
  {
    QUrl urlCopy(m_spData->m_sPath);
    QUrl baseUrl = QUrl::fromLocalFile(CApplication::Instance()->Settings()->ContentFolder() +
      "/" + sProjectName + "/");
    urlCopy.setScheme(QString());
    QUrl sFullPath = baseUrl.resolved(urlCopy);
    return sFullPath;
  }
  else
  {
    return m_spData->m_sPath;
  }
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
    return
      m_pEngine->newQObject(
          new CProject(m_pEngine, std::make_shared<SProject>(*m_spData->m_spParent)));
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
  return QStringList() << "*.json" << "*.xml" << "*.js" << "*.flow";
}

//----------------------------------------------------------------------------------------
//
QString ResourceUrlToAbsolutePath(const QUrl& url, const QString& sProjectFolder)
{
  if (url.isLocalFile())
  {
    QUrl urlCopy(url);
    QUrl baseUrl = QUrl::fromLocalFile(CApplication::Instance()->Settings()->ContentFolder() +
      "/" + sProjectFolder + "/");
    urlCopy.setScheme(QString());
    QUrl sFullPath = baseUrl.resolved(urlCopy);
    return sFullPath.toLocalFile();
  }
  else
  {
    return QString();
  }
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
