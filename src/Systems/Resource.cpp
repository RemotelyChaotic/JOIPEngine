#include "Resource.h"
#include "Application.h"
#include "PhysFs/PhysFsFileEngine.h"
#include "Project.h"

#include <QFileInfo>
#include <QImageReader>
#include <QMutexLocker>
#include <cassert>

//----------------------------------------------------------------------------------------
//
SResource::SResource(EResourceType type) :
  SResourceData(type),
  m_rwLock(QReadWriteLock::Recursive),
  m_spParent()
{}

SResource::SResource(const SResource& other) :
  SResourceData(other),
  m_rwLock(QReadWriteLock::Recursive),
  m_spParent(other.m_spParent),
  m_iLoadedId(other.m_iLoadedId)
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
        SResourceFormats::DatabaseFormats().contains("*." + sSuffix))
    {
      m_type = EResourceType::eDatabase;
    }
    else if (iValue == EResourceType::eOther &&
        SResourceFormats::ScriptFormats().contains("*." + sSuffix))
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
CResourceScriptWrapper::CResourceScriptWrapper(QJSEngine* pEngine, const std::shared_ptr<SResource>& spResource) :
  QObject(),
  CLockable(&spResource->m_rwLock),
  m_spData(spResource),
  m_pEngine(pEngine)
{
  assert(nullptr != spResource);
  assert(nullptr != pEngine);
}

CResourceScriptWrapper::~CResourceScriptWrapper()
{
}

//----------------------------------------------------------------------------------------
//
bool CResourceScriptWrapper::isAnimatedImpl()
{
  QReadLocker locker(&m_spData->m_rwLock);
  switch (m_spData->m_type)
  {
    case EResourceType::eImage:
    {
      if (IsLocalFile(m_spData->m_sPath))
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
bool CResourceScriptWrapper::isLocalPath()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return IsLocalFile(m_spData->m_sPath);
}

//----------------------------------------------------------------------------------------
//
QString CResourceScriptWrapper::getName()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sName;
}

//----------------------------------------------------------------------------------------
//
QUrl CResourceScriptWrapper::getPath()
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
  if (IsLocalFile(m_spData->m_sPath))
  {
    if (!bBundled)
    {
      QUrl urlCopy(m_spData->m_sPath);
      urlCopy.setScheme(QString());
      QString sBasePath = CPhysFsFileEngineHandler::c_sScheme;
      return QUrl(sBasePath + QUrl().resolved(urlCopy).toString());
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
QUrl CResourceScriptWrapper::getSource()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sSource;
}

//----------------------------------------------------------------------------------------
//
CResourceScriptWrapper::ResourceType CResourceScriptWrapper::getType()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return ResourceType(m_spData->m_type._to_integral());
}

//----------------------------------------------------------------------------------------
//
QJSValue CResourceScriptWrapper::project()
{
  QReadLocker locker(&m_spData->m_rwLock);
  if (nullptr != m_spData->m_spParent)
  {
    CProjectScriptWrapper* pProject =
        new CProjectScriptWrapper(m_pEngine, std::make_shared<SProject>(*m_spData->m_spParent));
    return m_pEngine->newQObject(pProject);
  }
  return QJSValue();
}

//----------------------------------------------------------------------------------------
//
QStringList SResourceFormats::AudioFormats()
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
bool IsLocalFile(const QUrl& url)
{
  return url.isLocalFile() ||
      CPhysFsFileEngineHandler::c_sScheme.contains(url.scheme());
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
  if (IsLocalFile(spResource->m_sPath))
  {
    if (!bBundled)
    {
      QUrl urlCopy(spResource->m_sPath);
      urlCopy.setScheme(QString());
      QString sBasePath = CPhysFsFileEngineHandler::c_sScheme;
      return sBasePath + QUrl().resolved(urlCopy).toString();
    }
    else
    {
      return "qrc:/" + sTruePathName + "/" + spResource->m_sName;
    }
  }
  else
  {
    return QString();
  }
}

//----------------------------------------------------------------------------------------
//
QString ResourceUrlToRelativePath(const tspResource& spResource)
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
  if (IsLocalFile(spResource->m_sPath))
  {
    if (!bBundled)
    {
      QUrl urlCopy(spResource->m_sPath);
      urlCopy.setScheme(QString());
      return QUrl().resolved(urlCopy).toString();
    }
    else
    {
      return spResource->m_sName;
    }
  }
  else
  {
    return QString();
  }
}

//----------------------------------------------------------------------------------------
//
QUrl ResourceUrlFromLocalFile(const QString& sPath)
{
  QUrl url;
  if (sPath.startsWith(CPhysFsFileEngineHandler::c_sScheme))
  {
    url = QUrl(QString(sPath).replace(CPhysFsFileEngineHandler::c_sScheme, ""));
  }
  else
  {
    url = QUrl::fromLocalFile(sPath);
  }
  url.setScheme(QString(CPhysFsFileEngineHandler::c_sScheme).replace(":/", ""));
  return url;
}

//----------------------------------------------------------------------------------------
//
QStringList SResourceFormats::ArchiveFormats()
{
  QStringList vsFormats = CPhysFsFileEngineHandler::SupportedFileTypes();
  static QStringList vsReturn;
  if (vsReturn.isEmpty())
  {
    for (const QString& sFormat : vsFormats) { vsReturn <<  QStringLiteral("*.") + sFormat.toLower(); }
  }
  return vsReturn;
}

//----------------------------------------------------------------------------------------
//
QStringList SResourceFormats::DatabaseFormats()
{
  static QStringList vsFormats = QStringList() << "*.json" << "*.xml";
  return vsFormats;
}

//----------------------------------------------------------------------------------------
//
QStringList SResourceFormats::FontFormats()
{
  return QStringList() << "*.tff" << "*.otf" << "*.otc" << "*.ttf" << "*.ttc" << "*.pfa" << "*.pfb" << "*.bdf" << "*.cff" << "*.fnt" << "*.pcf";
}

//----------------------------------------------------------------------------------------
//
QStringList SResourceFormats::ImageFormats()
{
  QList<QByteArray> imageFormats = QImageReader::supportedImageFormats();
  static QStringList imageFormatsList;
  if (imageFormatsList.isEmpty())
  {
    QString sImageFormats;
    for (const QByteArray& arr : imageFormats) { imageFormatsList += "*." + QString::fromUtf8(arr); }
  }
  return imageFormatsList;
}

//----------------------------------------------------------------------------------------
//
QStringList SResourceFormats::OtherFormats()
{
  static QStringList vsFormats = QStringList() << "*.json" << "*.proj" << "*.flow" << ".layout";
  return vsFormats;
}

//----------------------------------------------------------------------------------------
//
QStringList SResourceFormats::ScriptFormats()
{
  static QStringList vsFormats = QStringList() << "*.js";
  return vsFormats;
}

//----------------------------------------------------------------------------------------
//
QStringList SResourceFormats::VideoFormats()
{
  // TODO: check codecs
  static QStringList vsFormats = QStringList()
      << ".3gp" << "*.asf" << "*.wmv" << "*.mpg" << "*.ts"
         "*.au" << "*.avi" << "*.flv" << "*.mov" << "*.mp4" << "*.ogm" <<
         "*.ogg" << "*.mkv" << "*.mka" <<
         "*.nsc" << "*.nsv" << "*.nut" << "*.a52" <<
         "*.dv" << "*.vid" << "*.ty" << "*.webm";
  return vsFormats;
}


//----------------------------------------------------------------------------------------
//
QString SResourceFormats::JoinedFormatsForFilePicker()
{
  QString sFormatSelection = "Image Files (%1);;Video Files (%2);;Sound Files (%3);;Script Files (%4);;Archives (%5);;Fonts (%6);;Other Files (%7)";
  return sFormatSelection.arg(ImageFormats().join(" "))
                         .arg(VideoFormats().join(" "))
                         .arg(AudioFormats().join(" "))
                         .arg(ScriptFormats().join(" "))
                         .arg(ArchiveFormats().join(" "))
                         .arg(FontFormats().join(" "))
                         .arg(OtherFormats().join(" "));
}
