#include "Resource.h"
#include "Systems/PhysFs/PhysFsFileEngine.h"
#include "Project.h"

#include <QFileInfo>
#include <QImageReader>
#include <QJsonArray>
#include <QMutexLocker>
#include <cassert>

//----------------------------------------------------------------------------------------
//
namespace
{
  QString MakePathProjectLocal(const QString& sPath, const QString& sFolderName)
  {
    static QString sPfs = QString(CPhysFsFileEngineHandler::c_sScheme).replace(":/", ":");
    return QString(sPath).replace(sPfs, sPfs + sFolderName + "/");
  }

  QString MakePathSerialized(const QString& sPath, const QString& sFolderName)
  {
    static QString sPfs = QString(CPhysFsFileEngineHandler::c_sScheme).replace(":/", ":");
    return QString(sPath).replace(sPfs + sFolderName + "/", sPfs);
  }

  SResourcePath CreatePathFromString(const QString& sStr, const tspProject& spProject)
  {
    if (SResourcePath::IsLocalFileP(QUrl(sStr)))
    {
      QReadLocker projectLocker(&spProject->m_rwLock);
      QString sPath = MakePathProjectLocal(sStr, spProject->m_sFolderName);
      return SResourcePath(sPath);
    }
    return SResourcePath(sStr);
  }
}

//----------------------------------------------------------------------------------------
//
SResource::SResource(EResourceType type) :
  SResourceData(type),
  m_rwLock(QReadWriteLock::Recursive),
  m_spParent()
{}

SResource::SResource(const SResource& other) :
  std::enable_shared_from_this<SResource>(other),
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
  QJsonArray resourceTags;
  for (const QString& sTag : m_vsResourceTags)
  {
    resourceTags.push_back(sTag);
  }
  return {
    { "sName", m_sName },
    { "sPath", MakePathSerialized(static_cast<QString>(m_sPath), m_spParent->m_sFolderName) },
    { "sSource", m_sSource.toString(QUrl::None) },
    { "type", m_type._value },
    { "sResourceBundle", m_sResourceBundle },
    { "vsResourceTags", resourceTags }
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
    m_sPath = ::CreatePathFromString(it.value().toString(), m_spParent);
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
    Q_UNUSED(bBundled)
    locker.unlock();
    const QString sSuffix = m_sPath.Suffix();
    locker.relock();

    qint32 iValue = it.value().toInt();
    if (EResourceType::_size() > static_cast<size_t>(iValue))
    {
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
      else if (iValue == EResourceType::eOther &&
               SResourceFormats::LayoutFormats().contains("*." + sSuffix))
      {
        m_type = EResourceType::eLayout;
      }
      else
      {
        m_type = EResourceType::_from_integral(iValue);
      }
    }
    else
    {
      m_type = EResourceType::eOther;
    }
  }
  it = json.find("sResourceBundle");
  if (it != json.end())
  {
    m_sResourceBundle = it.value().toString();
  }
  it = json.find("vsResourceTags");
  m_vsResourceTags.clear();
  if (it != json.end())
  {
    for (const QJsonValue& val : it.value().toArray())
    {
      const QString sValue = val.toString();
      if (!sValue.isEmpty())
      {
        m_vsResourceTags.insert(sValue);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
QString SResource::PhysicalResourcePath() const
{
  QReadLocker locker(&m_rwLock);
  if ( nullptr == m_spParent)
  {
    return QString();
  }

  return joip_resource::PhysicalResourcePath(static_cast<QUrl>(m_sPath), m_spParent,
                                             m_sResourceBundle, m_sName);
}

//----------------------------------------------------------------------------------------
//
QString SResource::ResourceToAbsolutePath(const QString& sResourceScheme) const
{
  QReadLocker locker(&m_rwLock);
  if (nullptr == m_spParent)
  {
    return QString();
  }

  QReadLocker projectLocker(&m_spParent->m_rwLock);
  const QString sTrueProjectName = m_spParent->m_sName;
  bool bBundled = m_spParent->m_bBundled;
  projectLocker.unlock();

  if (m_sPath.IsLocalFile())
  {
    if (!bBundled && m_sResourceBundle.isEmpty())
    {
      QUrl urlCopy(static_cast<QUrl>(m_sPath));
      urlCopy.setScheme(QString());
      QString sBasePath = CPhysFsFileEngineHandler::c_sScheme;
      return sBasePath + QUrl().resolved(urlCopy).toString();
    }
    else
    {
      return sResourceScheme + "/" + sTrueProjectName + "/" + m_sName;
    }
  }
  else
  {
    return QString();
  }
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
SResourcePath joip_resource::CreatePathFromAbsolutePath(const QString& sStr, const tspProject& spProject)
{
  if (SResourcePath::IsLocalFileP(QUrl(sStr)))
  {
    QReadLocker projectLocker(&spProject->m_rwLock);
    QString sFolderName = spProject->m_sFolderName;
    QString sPhysPath = PhysicalProjectPath(spProject);

    QString sRelativePath = QString(CPhysFsFileEngineHandler::c_sScheme).replace(":/", ":") +
                            QDir(sPhysPath).relativeFilePath(sStr);

    QString sPath = MakePathProjectLocal(sRelativePath, sFolderName);
    return SResourcePath(sPath);
  }
  return SResourcePath(sStr);
}

//----------------------------------------------------------------------------------------
//
SResourcePath joip_resource::CreatePathFromAbsoluteUrl(const QUrl& sUrl, const tspProject& spProject)
{
  if (SResourcePath::IsLocalFileP(sUrl))
  {
    QReadLocker projectLocker(&spProject->m_rwLock);
    QString sFolderName = spProject->m_sFolderName;
    QString sPhysPath = PhysicalProjectPath(spProject);

    QString sRelativePath = QString(CPhysFsFileEngineHandler::c_sScheme).replace(":/", ":") +
                            QDir(sPhysPath).relativeFilePath(sUrl.toLocalFile());

    QString sPath = MakePathProjectLocal(sRelativePath, sFolderName);
    return SResourcePath(sPath);
  }
  return SResourcePath(sUrl);
}

//----------------------------------------------------------------------------------------
//
QString joip_resource::PhysicalResourcePath(const QUrl& url, const tspProject& spProject,
                                            const QString& sResourceBundle, const QString& sResourceName)
{
  QReadLocker projectLocker(&spProject->m_rwLock);
  const QString sTrueProjectName = spProject->m_sName;
  const QString sProjectFolder = spProject->m_sFolderName;
  bool bBundled = spProject->m_bBundled;
  projectLocker.unlock();

  if (SResourcePath::IsLocalFileP(url))
  {
    if (!bBundled && sResourceBundle.isEmpty())
    {
      QUrl urlCopy(MakePathProjectLocal(url.toString(QUrl::None), sProjectFolder));
      urlCopy.setScheme(QString());
      QString sBasePath = PhysicalProjectPath(spProject);
      return sBasePath + "/" + QUrl().resolved(urlCopy).toString();
    }
    else
    {
      return ":/" + sTrueProjectName + "/" + sResourceName;
    }
  }
  return url.toString(QUrl::None);
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
  // adding this kind of resorce doesn't work, but it can be read and will be created by
  // the engine itself if needed
  //vsReturn << ("*." + joip_resource::c_sResourceBundleSuffix);
  return vsReturn;
}

//----------------------------------------------------------------------------------------
//
QStringList SResourceFormats::DatabaseFormats()
{
  static QStringList vsFormats = QStringList() << "*.json" << "*.xml" <<
                                 ("*." + joip_resource::c_sDialogueFileType);
  return vsFormats;
}

//----------------------------------------------------------------------------------------
//
QStringList SResourceFormats::FontFormats()
{
  return QStringList() << "*.tff" << "*.otf" << "*.otc" << "*.ttf"; // << "*.ttc" << "*.pfa" << "*.pfb" << "*.bdf" << "*.cff" << "*.fnt" << "*.pcf";
}

//----------------------------------------------------------------------------------------
//
QStringList SResourceFormats::ImageFormats()
{
  QList<QByteArray> imageFormats = QImageReader::supportedImageFormats();
  static QStringList imageFormatsList;
  if (imageFormatsList.isEmpty())
  {
    for (const QByteArray& arr : imageFormats) { imageFormatsList += "*." + QString::fromUtf8(arr); }
  }
  return imageFormatsList;
}

//----------------------------------------------------------------------------------------
//
QStringList SResourceFormats::LayoutFormats()
{
  static QStringList vsFormats = QStringList() << "*.qml" << "*.layout";
  return vsFormats;
}

//----------------------------------------------------------------------------------------
//
QStringList SResourceFormats::OtherFormats()
{
  static QStringList vsFormats = QStringList() << "*.json" << "*.proj" << "*.flow" << "*qmldir";
  return vsFormats;
}

//----------------------------------------------------------------------------------------
//
QStringList SResourceFormats::ScriptFormats()
{
  static QStringList vsFormats = QStringList() << "*.js" << "*.eos" << "*.lua";
  return vsFormats;
}

//----------------------------------------------------------------------------------------
//
QStringList SResourceFormats::SequenceFormat()
{
  static QStringList vsFormats = QStringList() << "*.jseq";
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
  QString sFormatSelection = "Image Files (%1);;Video Files (%2);;Sound Files (%3);;Script Files (%4);;Archives (%5);;Fonts (%6);;Database Files (%7);;Other Files (%8)";
  return sFormatSelection.arg(ImageFormats().join(" "))
                         .arg(VideoFormats().join(" "))
                         .arg(AudioFormats().join(" "))
                         .arg(ScriptFormats().join(" "))
                         .arg(ArchiveFormats().join(" "))
                         .arg(FontFormats().join(" "))
                         .arg(DatabaseFormats().join(" "))
                         .arg(OtherFormats().join(" "));
}
