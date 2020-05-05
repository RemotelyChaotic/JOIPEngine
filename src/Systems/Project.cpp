#include "Project.h"
#include "Application.h"
#include "DatabaseManager.h"
#include "SVersion.h"
#include "version.h"
#include <QApplication>
#include <QJsonArray>
#include <QMutexLocker>
#include <cassert>

namespace  {
  const QRegExp c_rxExcludedNames("^(LPT1|LPT2|LPT3|LPT4|LPT5|LPT6|LPT7|LPT8|LPT9|COM1|COM2|COM3|COM4|COM5|COM6|COM7|COM8|COM9|PRN|AUX|NUL|CON|CLOCK\\$|\\.|\\.\\.)$");
}

//----------------------------------------------------------------------------------------
//
SProject::SProject() :
  m_rwLock(QReadWriteLock::Recursive),
  m_iId(),
  m_iVersion(SVersion(1, 0, 0)),
  m_iTargetVersion(SVersion(MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION)),
  m_sName(),
  m_sFolderName(),
  m_sDescribtion(),
  m_sTitleCard(),
  m_sMap(),
  m_sSceneModel(),
  m_bUsesWeb(false),
  m_bNeedsCodecs(false),
  m_vsKinks(),
  m_vspScenes(),
  m_spResourcesMap()
{}
SProject::SProject(const SProject& other) :
  m_rwLock(QReadWriteLock::Recursive),
  m_iId(other.m_iId),
  m_iVersion(other.m_iVersion),
  m_iTargetVersion(other.m_iTargetVersion),
  m_sName(other.m_sName),
  m_sFolderName(other.m_sFolderName),
  m_sDescribtion(other.m_sDescribtion),
  m_sTitleCard(other.m_sTitleCard),
  m_sMap(other.m_sMap),
  m_sSceneModel(other.m_sSceneModel),
  m_bUsesWeb(other.m_bUsesWeb),
  m_bNeedsCodecs(other.m_bNeedsCodecs),
  m_vsKinks(other.m_vsKinks),
  m_vspScenes(other.m_vspScenes),
  m_spResourcesMap(other.m_spResourcesMap)
{}

SProject::~SProject() {}

//----------------------------------------------------------------------------------------
//
QJsonObject SProject::ToJsonObject()
{
  QWriteLocker locker(&m_rwLock);

  QJsonArray kinks;
  for (const QString& sKink : m_vsKinks)
  {
    kinks.push_back(sKink);
  }
  QJsonArray scenes;
  for (auto& spScene : m_vspScenes)
  {
    scenes.push_back(spScene->ToJsonObject());
  }
  QJsonArray resources;
  for (auto& spResource : m_spResourcesMap)
  {
    resources.push_back(spResource.second->ToJsonObject());
  }
  return {
    { "iVersion", static_cast<qint32>(static_cast<quint32>(m_iVersion)) },
    { "iTargetVersion", static_cast<qint32>(static_cast<quint32>(m_iTargetVersion)) },
    { "sName", m_sName },
    { "sDescribtion", m_sDescribtion },
    { "sTitleCard", m_sTitleCard },
    { "sMap", m_sMap },
    { "sSceneModel", m_sSceneModel },
    { "bUsesWeb", m_bUsesWeb },
    { "bNeedsCodecs", m_bNeedsCodecs },
    { "vsKinks", kinks },
    { "vspScenes", scenes },
    { "vspResources", resources },
  };
}

//----------------------------------------------------------------------------------------
//
void SProject::FromJsonObject(const QJsonObject& json)
{
  QWriteLocker locker(&m_rwLock);
  auto it = json.find("iVersion");
  if (it != json.end())
  {
    m_iVersion = static_cast<quint32>(it.value().toInt());
  }
  it = json.find("iTargetVersion");
  if (it != json.end())
  {
    m_iTargetVersion = static_cast<quint32>(it.value().toInt());
  }
  it = json.find("sName");
  if (it != json.end())
  {
    m_sName = it.value().toString();
  }
  it = json.find("sDescribtion");
  if (it != json.end())
  {
    m_sDescribtion = it.value().toString();
  }
  it = json.find("sTitleCard");
  if (it != json.end())
  {
    m_sTitleCard = it.value().toString();
  }
  it = json.find("sMap");
  if (it != json.end())
  {
    m_sMap = it.value().toString();
  }
  it = json.find("sSceneModel");
  if (it != json.end())
  {
    m_sSceneModel = it.value().toString();
  }
  it = json.find("bUsesWeb");
  if (it != json.end())
  {
    m_bUsesWeb = it.value().toBool();
  }
  it = json.find("bNeedsCodecs");
  if (it != json.end())
  {
    m_bNeedsCodecs = it.value().toBool();
  }
  it = json.find("vsKinks");
  m_vsKinks.clear();
  if (it != json.end())
  {
    for (QJsonValue val : it.value().toArray())
    {
      QString sKink = val.toString();
      m_vsKinks.push_back(sKink);
    }
  }
  it = json.find("vspScenes");
  m_vspScenes.clear();
  if (it != json.end())
  {
    for (QJsonValue val : it.value().toArray())
    {
      std::shared_ptr<SScene> spScene = std::make_shared<SScene>();
      spScene->FromJsonObject(val.toObject());
      spScene->m_spParent = GetPtr();
      m_vspScenes.push_back(spScene);
    }
  }
  it = json.find("vspResources");
  m_spResourcesMap.clear();
  if (it != json.end())
  {
    for (QJsonValue val : it.value().toArray())
    {
      std::shared_ptr<SResource> spResource = std::make_shared<SResource>(EResourceType::eOther);
      spResource->FromJsonObject(val.toObject());
      spResource->m_spParent = GetPtr();
      m_spResourcesMap.insert({spResource->m_sName, spResource});
    }
  }
}

//----------------------------------------------------------------------------------------
//
CProject::CProject(QJSEngine* pEngine, const std::shared_ptr<SProject>& spProject) :
  QObject(),
  m_spData(spProject),
  m_pEngine(pEngine)
{
  assert(nullptr != spProject);
  assert(nullptr != pEngine);
}

CProject::~CProject()
{
}

//----------------------------------------------------------------------------------------
//
qint32 CProject::getId()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_iId;
}

//----------------------------------------------------------------------------------------
//
qint32 CProject::getVersion()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return static_cast<qint32>(m_spData->m_iVersion);
}

//----------------------------------------------------------------------------------------
//
QString CProject::getVersionText()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return static_cast<QString>(m_spData->m_iVersion);
}

//----------------------------------------------------------------------------------------
//
qint32 CProject::getTargetVersion()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return static_cast<qint32>(m_spData->m_iTargetVersion);
}

//----------------------------------------------------------------------------------------
//
QString CProject::getTargetVersionText()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return static_cast<QString>(m_spData->m_iTargetVersion);
}

//----------------------------------------------------------------------------------------
//
QString CProject::getName()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sName;
}

//----------------------------------------------------------------------------------------
//
QString CProject::getFolderName()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sFolderName;
}

//----------------------------------------------------------------------------------------
//
QString CProject::getDescribtion()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sDescribtion;
}

//----------------------------------------------------------------------------------------
//
QString CProject::getTitleCard()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sTitleCard;
}

//----------------------------------------------------------------------------------------
//
QString CProject::getMap()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sMap;
}

//----------------------------------------------------------------------------------------
//
QStringList CProject::getKinks()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_vsKinks;
}

//----------------------------------------------------------------------------------------
//
bool CProject::isUsingWeb()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_bUsesWeb;
}

//----------------------------------------------------------------------------------------
//
bool CProject::isUsingCodecs()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_bNeedsCodecs;
}

//----------------------------------------------------------------------------------------
//
qint32 CProject::numScenes()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return static_cast<qint32>(m_spData->m_vspScenes.size());
}

//----------------------------------------------------------------------------------------
//
QJSValue CProject::scene(qint32 iIndex)
{
  QReadLocker locker(&m_spData->m_rwLock);
  if (0 <= iIndex && m_spData->m_vspScenes.size() > static_cast<size_t>(iIndex))
  {
    return
      m_pEngine->newQObject(
          new CScene(m_pEngine, std::make_shared<SScene>(*m_spData->m_vspScenes[static_cast<size_t>(iIndex)])));
  }
  return QJSValue();
}

//----------------------------------------------------------------------------------------
//
qint32 CProject::numResources()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return static_cast<qint32>(m_spData->m_spResourcesMap.size());
}

//----------------------------------------------------------------------------------------
//
QJSValue CProject::resource(const QString& sValue)
{
  QReadLocker locker(&m_spData->m_rwLock);
  auto it = m_spData->m_spResourcesMap.find(sValue);
  if (m_spData->m_spResourcesMap.end() != it)
  {
    return
      m_pEngine->newQObject(
          new CResource(m_pEngine, std::make_shared<SResource>(*it->second)));
  }
  return QJSValue();
}

//----------------------------------------------------------------------------------------
//
QString PhysicalProjectName(const tspProject& spProject)
{
  QReadLocker locker(&spProject->m_rwLock);
  QString sName = spProject->m_sName;
  if (!spProject->m_sFolderName.isNull())
  {
    sName = spProject->m_sFolderName;
  }
  return sName;
}

//----------------------------------------------------------------------------------------
//
bool ProjectNameCheck(const QString& sProjectName, QString* sErrorText)
{
  auto spDbManager = CApplication::Instance()->System<CDatabaseManager>().lock();
  if (nullptr == spDbManager) { return false; }

  if (sProjectName.isEmpty() || sProjectName.isNull())
  {
    if (nullptr != sErrorText)
    {
      *sErrorText = QString(QApplication::translate("ProjectNameCheck",
                                                    QT_TR_NOOP("Project name cannot be empty.")));
    }
    return false;
  }
  else if (255 < sProjectName.length())
  {
    if (nullptr != sErrorText)
    {
      *sErrorText = QString(QApplication::translate("ProjectNameCheck",
                                                    QT_TR_NOOP("Project name must have at most 255 characters.")));
    }
    return false;
  }
  else if (QRegExp("^\\s*$").exactMatch(sProjectName))
  {
    if (nullptr != sErrorText)
    {
      *sErrorText = QString(QApplication::translate("ProjectNameCheck",
                                                    QT_TR_NOOP("Project name must contain at least one non-whitespace character.")));
    }
    return false;
  }
  else if (sProjectName.contains(QRegExp("\"|\\\\|\\/|:|\\||\\<|\\>|\\*|\\?|&|\\(|\\)|\\+|\\[|\\]|;")))
  {
    if (nullptr != sErrorText)
    {
      *sErrorText = QString(QApplication::translate("ProjectNameCheck",
                                                    QT_TR_NOOP("Characters: \" \\ / : | < > * ? & ( ) ; + [ ] not allowed in project name.")));
    }
    return false;
  }
  else if (!sProjectName[0].isLetterOrNumber())
  {
    if (nullptr != sErrorText)
    {
      *sErrorText = QString(QApplication::translate("ProjectNameCheck",
                                                    QT_TR_NOOP("Project name must begin with a letter or number.")));
    }
    return false;
  }
  else if (sProjectName.contains(c_rxExcludedNames))
  {
    if (nullptr != sErrorText)
    {
      *sErrorText = QString(QApplication::translate("ProjectNameCheck",
                                                    QT_TR_NOOP("The following project names are not allowed: LPT1, LPT2, LPT3, LPT4, LPT5, LPT6, LPT7, LPT8, LPT9, COM1, COM2, COM3, COM4, COM5, COM6, COM7, COM8, COM9, PRN, AUX, NUL, CON, CLOCK$, dot character (.), and two dot characters (..)")));
    }
    return false;
  }
  else if (nullptr != spDbManager && spDbManager->FindProject(sProjectName) != nullptr)
  {
    if (nullptr != sErrorText)
    {
      *sErrorText = QString(QApplication::translate("ProjectNameCheck",
                                                    QT_TR_NOOP("Project with the name '%1' allready exists.")));
    }
    return false;
  }
  else
  {
    QString sIllegalChars;
    for (const QChar& c : sProjectName)
    {
      if (0x1f > c.unicode() || 0x80 <= c.unicode() ||
          !(c.isLetterOrNumber() || c.isSymbol() || c.isMark() || c.isSpace() || c.isPunct()))
      {
        sIllegalChars += c;
      }
    }
    if (0 < sIllegalChars.length())
    {
      if (nullptr != sErrorText)
      {
        *sErrorText = QString(QApplication::translate("ProjectNameCheck",
                                                      QT_TR_NOOP("Illegal unicode character(s) '%1' in project name."))).arg(sIllegalChars);
      }
      return false;
    }
  }

  return true;
}

//----------------------------------------------------------------------------------------
//
QString ToValidProjectName(const QString& sProjectName)
{
  auto spDbManager = CApplication::Instance()->System<CDatabaseManager>().lock();
  if (nullptr == spDbManager) { return QString(); }
  QString sFinalName = sProjectName;

  if (sProjectName.isEmpty() || sProjectName.isNull() || QRegExp("^\\s*$").exactMatch(sProjectName))
  {
    sFinalName = "UnnamedPoject";
  }
  else
  {
    sFinalName = sProjectName;
  }
  if (255 < sFinalName.length())
  {
    sFinalName = sFinalName.mid(0, 255);
  }
  if (!sFinalName[0].isLetterOrNumber())
  {
    sFinalName.replace(0, 1, '0');
  }
  if (sFinalName.contains(c_rxExcludedNames) ||
      (nullptr != spDbManager && spDbManager->FindProject(sProjectName) != nullptr))
  {
    QString sOrigName = sFinalName;
    qint32 iCounter = 0;
    while (sFinalName.contains(c_rxExcludedNames) ||
           (nullptr != spDbManager && spDbManager->FindProject(sFinalName) != nullptr))
    {
      sFinalName = sOrigName + QString::number(iCounter);
      iCounter++;
    }
  }

  for (qint32 i = 0; sFinalName.length() > i; ++i)
  {
    QChar c = sFinalName[i];
    if (0x1f > c.unicode() || 0x80 <= c.unicode() ||
        !(c.isLetterOrNumber() || c.isSymbol() || c.isMark() || c.isSpace() || c.isPunct()))
    {
      sFinalName.replace(i, 1, '0');
    }
  }

  return sFinalName;
}
