#include "Project.h"
#include "Application.h"
#include "Kink.h"
#include "SVersion.h"
#include "out_Version.h"

#include "Systems/DatabaseManager.h"

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
  SProjectData(),
  m_rwLock(QReadWriteLock::Recursive),
  m_vsKinks(),
  m_vspScenes(),
  m_spResourcesMap(),
  m_spResourceBundleMap(),
  m_vspTags(),
  m_vspAchievements(),
  m_vsMountPoints()
{
}
SProject::SProject(const SProject& other) :
  std::enable_shared_from_this<SProject>(other),
  SProjectData(other),
  m_rwLock(QReadWriteLock::Recursive),
  m_vsKinks(other.m_vsKinks),
  m_vspScenes(other.m_vspScenes),
  m_spResourcesMap(other.m_spResourcesMap),
  m_spResourceBundleMap(other.m_spResourceBundleMap),
  m_vspTags(other.m_vspTags),
  m_vspAchievements(other.m_vspAchievements),
  m_vsMountPoints(other.m_vsMountPoints)
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
  QJsonArray resourceBundles;
  for (auto& spResourceBundle : m_spResourceBundleMap)
  {
    resourceBundles.push_back(spResourceBundle.second->ToJsonObject());
  }
  QJsonArray tags;
  for (auto& spTag : m_vspTags)
  {
    tags.push_back(spTag.second->ToJsonObject());
  }
  QJsonArray achievements;
  for (auto& spAchievement : m_vspAchievements)
  {
    achievements.push_back(spAchievement.second->ToJsonObject());
  }
  QJsonArray mountPoints;
  for (auto& sMountPoint : m_vsMountPoints)
  {
    mountPoints.push_back(sMountPoint);
  }
  return {
    { "iVersion", static_cast<qint32>(static_cast<quint32>(m_iVersion)) },
    { "iTargetVersion", static_cast<qint32>(static_cast<quint32>(m_iTargetVersion)) },
    { "sName", m_sName },
    { "sDescribtion", m_sDescribtion },
    { "sTitleCard", m_sTitleCard },
    { "sMap", m_sMap },
    { "sSceneModel", m_sSceneModel },
    { "sPlayerLayout", m_sPlayerLayout },
    { "tutorialState", m_tutorialState._to_integral() },
    { "iNumberOfSoundEmitters", m_iNumberOfSoundEmitters },
    { "metCmdMode", m_metCmdMode },
    { "bCanStartAtAnyScene", m_bCanStartAtAnyScene },
    { "bUsesWeb", m_bUsesWeb },
    { "bNeedsCodecs", m_bNeedsCodecs },
    { "vsKinks", kinks },
    { "vspScenes", scenes },
    { "vspResources", resources },
    { "vspResourceBundles", resourceBundles },
    { "vspTags", tags },
    { "vspAchievements", achievements },
    { "vsMountPoints", mountPoints },
    { "dlState", m_dlState._to_integral() },
    { "sFont", m_sFont },
    { "sPluginFolder", m_sPluginFolder },
    { "sUserData", m_sUserData }
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
  it = json.find("sPlayerLayout");
  if (it != json.end())
  {
    m_sPlayerLayout = it.value().toString();
    if (m_sPlayerLayout.isEmpty() ||
        "qrc:/qml/resources/qml/PlayerDefaultLayout.qml" == m_sPlayerLayout)
    {
      m_sPlayerLayout = "qrc:/qml/resources/qml/JoipEngine/PlayerDefaultLayoutClassic.qml";
    }
  }
  it = json.find("tutorialState");
  if (it != json.end())
  {
    qint32 iVal = it.value().toInt();
    if (ETutorialState::_is_valid(iVal))
    {
      m_tutorialState = ETutorialState::_from_integral(iVal);
    }
  }
  it = json.find("iNumberOfSoundEmitters");
  if (it != json.end())
  {
    m_iNumberOfSoundEmitters = it.value().toInt();
  }
  it = json.find("metCmdMode");
  if (it != json.end())
  {
    m_metCmdMode = it.value().toInt();
  }
  it = json.find("bCanStartAtAnyScene");
  if (it != json.end())
  {
    m_bCanStartAtAnyScene = it.value().toBool();
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
      spScene->m_spParent = GetPtr();
      locker.unlock();
      spScene->FromJsonObject(val.toObject());
      locker.relock();
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
      spResource->m_spParent = GetPtr();
      locker.unlock();
      spResource->FromJsonObject(val.toObject());
      locker.relock();
      m_spResourcesMap.insert({spResource->m_sName, spResource});
    }
  }
  it = json.find("vspResourceBundles");
  m_spResourceBundleMap.clear();
  if (it != json.end())
  {
    for (QJsonValue val : it.value().toArray())
    {
      std::shared_ptr<SResourceBundle> spResourceBundle =
          std::make_shared<SResourceBundle>();
      spResourceBundle->m_spParent = GetPtr();
      locker.unlock();
      spResourceBundle->FromJsonObject(val.toObject());
      locker.relock();
      m_spResourceBundleMap.insert({spResourceBundle->m_sName, spResourceBundle});
    }
  }
  it = json.find("vspTags");
  m_vspTags.clear();
  if (it != json.end())
  {
    for (QJsonValue val : it.value().toArray())
    {
      tspTag spTag = std::make_shared<STag>();
      spTag->m_spParent = GetPtr();
      locker.unlock();
      spTag->FromJsonObject(val.toObject());
      locker.relock();
      m_vspTags.insert(std::pair<QString, tspTag>{spTag->m_sName, spTag});
    }
  }
  it = json.find("vspAchievements");
  m_vspAchievements.clear();
  if (it != json.end())
  {
    for (QJsonValue val : it.value().toArray())
    {
      tspSaveData spAchievement = std::make_shared<SSaveData>();
      spAchievement->m_spParent = GetPtr();
      locker.unlock();
      spAchievement->FromJsonObject(val.toObject());
      locker.relock();
      m_vspAchievements.insert(std::pair<QString, tspSaveData>{spAchievement->m_sName, spAchievement});
    }
  }

  // connect tags and resources after both have been loaded
  for (const auto& [sResource, spResource] : m_spResourcesMap)
  {
    QReadLocker resLocker(&spResource->m_rwLock);
    for (const QString& sTag : spResource->m_vsResourceTags)
    {
      auto it = m_vspTags.find(sTag);
      if (m_vspTags.end() != it)
      {
        QReadLocker tagLocker(&it->second->m_rwLock);
        it->second->m_vsResourceRefs.insert(sResource);
      }
    }
  }

  it = json.find("vsMountPoints");
  m_vsMountPoints.clear();
  if (it != json.end())
  {
    for (QJsonValue val : it.value().toArray())
    {
      QString sMountPoint = val.toString();
      m_vsMountPoints.push_back(sMountPoint);
    }
  }
  it = json.find("dlState");
  if (it != json.end())
  {
    qint32 iValue = it.value().toInt();
    if (EDownLoadState::_is_valid(iValue))
    {
      m_dlState = EDownLoadState::_from_integral(iValue);
    }
  }
  it = json.find("sFont");
  if (it != json.end())
  {
    m_sFont = it.value().toString();
  }
  it = json.find("sPluginFolder");
  if (it != json.end())
  {
    m_sPluginFolder = it.value().toString();
  }
  it = json.find("sUserData");
  if (it != json.end())
  {
    m_sUserData = it.value().toString();
  }
}

//----------------------------------------------------------------------------------------
//
QString PhysicalProjectName(const tspProject& spProject)
{
  QReadLocker locker(&spProject->m_rwLock);
  QString sName = spProject->m_sName;
  if (!spProject->m_sFolderName.isNull() && sName != spProject->m_sFolderName)
  {
    sName = spProject->m_sFolderName;
  }
  return sName;
}

//----------------------------------------------------------------------------------------
//
QString PhysicalProjectPath(const tspProject& spProject)
{
  QString sFolderName = PhysicalProjectName(spProject);
  QReadLocker locker(&spProject->m_rwLock);
  return spProject->m_sProjectPath + "/" + sFolderName;
}

//----------------------------------------------------------------------------------------
//
bool ProjectNameCheck(const QString& sProjectName, QString* sErrorText,
                      const tspProject& spExclude)
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
  else if (auto spProj = spDbManager->FindProject(sProjectName);
           spProj != nullptr && spExclude != spProj)
  {
    if (nullptr != sErrorText)
    {
      *sErrorText = QString(QApplication::translate("ProjectNameCheck",
                                                    QT_TR_NOOP("Project with the name '%1' allready exists.")))
                      .arg(sProjectName);
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
