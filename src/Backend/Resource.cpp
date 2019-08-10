#include "Resource.h"
#include "Application.h"
#include "Project.h"

#include <QImageReader>
#include <QMutexLocker>
#include <QScriptEngine>
#include <cassert>

SResource::SResource(EResourceType type) :
  m_spParent(nullptr),
  m_rwLock(QReadWriteLock::Recursive),
  m_sPath(),
  m_type(type)
{}

SResource::SResource(const SResource& other) :
  m_spParent(other.m_spParent),
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
  return m_spData->m_sPath.toString(QUrl::None);
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
tspProjectRef CResource::Project()
{
  QReadLocker locker(&m_spData->m_rwLock);
  if (nullptr != m_spData->m_spParent)
  {
    return tspProjectRef(new CProject(std::make_shared<SProject>(*m_spData->m_spParent)));
  }
  return nullptr;
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
  return QStringList() << "*.json" << "*.js";
}

//----------------------------------------------------------------------------------------
//
QString ResourceUrlToAbsolutePath(const QUrl& url, const QString& sProjectName)
{
  if (url.isLocalFile())
  {
    QUrl urlCopy(url);
    QUrl baseUrl = QUrl::fromLocalFile(CApplication::Instance()->Settings()->ContentFolder() +
      "/" + sProjectName + "/");
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
