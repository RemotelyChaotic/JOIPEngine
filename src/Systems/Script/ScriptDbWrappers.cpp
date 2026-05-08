#include "ScriptDbWrappers.h"
#include "Application.h"

#include "Systems/Database/Tag.h"
#include "Utils/MetronomeHelpers.h"

#include <QtLua/State>
#include <QImageReader>
#include <QJSEngine>

//----------------------------------------------------------------------------------------
//
namespace
{
  QVariant CreateScriptObject(QObject* pObjFrom,
                              tEngineType pEngine)
  {
    if (std::holds_alternative<QJSEngine*>(pEngine))
    {
      return std::get<QJSEngine*>(pEngine)->newQObject(pObjFrom).toVariant();
    }
    else if (std::holds_alternative<QtLua::State*>(pEngine))
    {
      return QtLua::Value(std::get<QtLua::State*>(pEngine),
                          pObjFrom, true, false);
    }
    return QVariant();
  }

  //--------------------------------------------------------------------------------------
  //
  [[maybe_unused]] bool CheckEngineNotNull(tEngineType pEngine)
  {
    if (std::holds_alternative<QJSEngine*>(pEngine))
    {
      return nullptr != std::get<QJSEngine*>(pEngine);
    }
    else if (std::holds_alternative<QtLua::State*>(pEngine))
    {
      return nullptr != std::get<QtLua::State*>(pEngine);
    }
    return false;
  }
}

//----------------------------------------------------------------------------------------
//
CProjectScriptWrapperReadOnly::CProjectScriptWrapperReadOnly(tEngineType pEngine, const std::shared_ptr<SProject>& spProject) :
  QObject(),
  CLockable(&spProject->m_rwLock),
  m_spData(spProject),
  m_pEngine(pEngine)
{
  assert(nullptr != spProject);
  assert(CheckEngineNotNull(pEngine));
}

CProjectScriptWrapperReadOnly::~CProjectScriptWrapperReadOnly()
{
}

//----------------------------------------------------------------------------------------
//
qint32 CProjectScriptWrapperReadOnly::getId()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_iId;
}

//----------------------------------------------------------------------------------------
//
qint32 CProjectScriptWrapperReadOnly::getVersion()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return static_cast<qint32>(m_spData->m_iVersion);
}

//----------------------------------------------------------------------------------------
//
QString CProjectScriptWrapperReadOnly::getVersionText()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return static_cast<QString>(m_spData->m_iVersion);
}

//----------------------------------------------------------------------------------------
//
qint32 CProjectScriptWrapperReadOnly::getTargetVersion()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return static_cast<qint32>(m_spData->m_iTargetVersion);
}

//----------------------------------------------------------------------------------------
//
QString CProjectScriptWrapperReadOnly::getTargetVersionText()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return static_cast<QString>(m_spData->m_iTargetVersion);
}

//----------------------------------------------------------------------------------------
//
QString CProjectScriptWrapperReadOnly::getName()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sName;
}

//----------------------------------------------------------------------------------------
//
QString CProjectScriptWrapperReadOnly::getFolderName()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sFolderName;
}

//----------------------------------------------------------------------------------------
//
QString CProjectScriptWrapperReadOnly::getDescribtion()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sDescribtion;
}

//----------------------------------------------------------------------------------------
//
QString CProjectScriptWrapperReadOnly::getTitleCard()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sTitleCard;
}

//----------------------------------------------------------------------------------------
//
QString CProjectScriptWrapperReadOnly::getMap()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sMap;
}

//----------------------------------------------------------------------------------------
//
QString CProjectScriptWrapperReadOnly::getSceneModel()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sSceneModel;
}

//----------------------------------------------------------------------------------------
//
QString CProjectScriptWrapperReadOnly::getPlayerLayout()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sPlayerLayout;
}

//----------------------------------------------------------------------------------------
//
qint32 CProjectScriptWrapperReadOnly::getNumberOfSoundEmitters()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_iNumberOfSoundEmitters;
}

//----------------------------------------------------------------------------------------
//
qint32 CProjectScriptWrapperReadOnly::getMetCmdMode()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return metronome::MapCmdModeToFlags(m_spData->m_metCmdMode);
}

//----------------------------------------------------------------------------------------
//
bool CProjectScriptWrapperReadOnly::getCanStartAtAnyScene()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_bCanStartAtAnyScene;
}

//----------------------------------------------------------------------------------------
//
bool CProjectScriptWrapperReadOnly::isUsingWeb()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_bUsesWeb;
}

//----------------------------------------------------------------------------------------
//
bool CProjectScriptWrapperReadOnly::isUsingCodecs()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_bNeedsCodecs;
}

//----------------------------------------------------------------------------------------
//
bool CProjectScriptWrapperReadOnly::isBundled()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_bBundled;
}

//----------------------------------------------------------------------------------------
//
bool CProjectScriptWrapperReadOnly::isReadOnly()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_bReadOnly;
}

//----------------------------------------------------------------------------------------
//
bool CProjectScriptWrapperReadOnly::isLoaded()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_bLoaded;
}

//----------------------------------------------------------------------------------------
//
qint32 CProjectScriptWrapperReadOnly::getDlState()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_dlState._to_integral();
}

//----------------------------------------------------------------------------------------
//
QString CProjectScriptWrapperReadOnly::getFont()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sFont;
}

//----------------------------------------------------------------------------------------
//
QString CProjectScriptWrapperReadOnly::getUserData()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sUserData;
}

//----------------------------------------------------------------------------------------
//
qint32 CProjectScriptWrapperReadOnly::numKinks()
{
  QReadLocker locker(&m_spData->m_rwLock);
  QStringList vsKinksOut;
  vsKinksOut << m_spData->m_pluginData.m_vsKinks;
  vsKinksOut << m_spData->m_baseData.m_vsKinks;
  vsKinksOut.removeDuplicates();
  return vsKinksOut.size();
}

//----------------------------------------------------------------------------------------
//
QStringList CProjectScriptWrapperReadOnly::kinks()
{
  QReadLocker locker(&m_spData->m_rwLock);
  QStringList vsKinksOut;
  vsKinksOut << m_spData->m_pluginData.m_vsKinks;
  vsKinksOut << m_spData->m_baseData.m_vsKinks;
  vsKinksOut.removeDuplicates();
  return vsKinksOut;
}

//----------------------------------------------------------------------------------------
//
QVariant CProjectScriptWrapperReadOnly::kink(const QString& sName)
{
  QReadLocker locker(&m_spData->m_rwLock);
  auto it = std::find(m_spData->m_baseData.m_vsKinks.begin(), m_spData->m_baseData.m_vsKinks.end(), sName);
  if (m_spData->m_baseData.m_vsKinks.end() != it)
  {
    auto wpDbManager = CApplication::Instance()->System<CDatabaseManager>().lock();
    if (nullptr != wpDbManager)
    {
      tspKink spKink = wpDbManager->FindKink(sName);
      if (nullptr != spKink)
      {
        CKinkWrapperReadOnly* pKink = new CKinkWrapperReadOnly(m_pEngine, spKink);
        return CreateScriptObject(pKink, m_pEngine);
      }
    }
  }
  it = std::find(m_spData->m_pluginData.m_vsKinks.begin(), m_spData->m_pluginData.m_vsKinks.end(), sName);
  if (m_spData->m_pluginData.m_vsKinks.end() != it)
  {
    auto wpDbManager = CApplication::Instance()->System<CDatabaseManager>().lock();
    if (nullptr != wpDbManager)
    {
      tspKink spKink = wpDbManager->FindKink(sName);
      if (nullptr != spKink)
      {
        CKinkWrapperReadOnly* pKink = new CKinkWrapperReadOnly(m_pEngine, spKink);
        return CreateScriptObject(pKink, m_pEngine);
      }
    }
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
qint32 CProjectScriptWrapperReadOnly::numScenes()
{
  QReadLocker locker(&m_spData->m_rwLock);
  std::set<QString> vScenes;
  for (const auto& spScene : m_spData->m_pluginData.m_vspScenes)
  {
    QReadLocker l(&spScene->m_rwLock);
    vScenes.insert(spScene->m_sName);
  }
  for (const auto& spScene : m_spData->m_baseData.m_vspScenes)
  {
    QReadLocker l(&spScene->m_rwLock);
    vScenes.insert(spScene->m_sName);
  }
  return static_cast<qint32>(vScenes.size());
}

//----------------------------------------------------------------------------------------
//
QStringList CProjectScriptWrapperReadOnly::scenes()
{
  QStringList outList;
  QReadLocker locker(&m_spData->m_rwLock);
  for (qint32 iIndex = 0; m_spData->m_pluginData.m_vspScenes.size() > static_cast<size_t>(iIndex); ++iIndex)
  {
    QReadLocker sceneLocker(&m_spData->m_pluginData.m_vspScenes[static_cast<size_t>(iIndex)]->m_rwLock);
    outList << m_spData->m_pluginData.m_vspScenes[static_cast<size_t>(iIndex)]->m_sName;
  }
  for (qint32 iIndex = 0; m_spData->m_baseData.m_vspScenes.size() > static_cast<size_t>(iIndex); ++iIndex)
  {
    QReadLocker sceneLocker(&m_spData->m_baseData.m_vspScenes[static_cast<size_t>(iIndex)]->m_rwLock);
    outList << m_spData->m_baseData.m_vspScenes[static_cast<size_t>(iIndex)]->m_sName;
  }
  outList.removeDuplicates();
  return outList;
}

//----------------------------------------------------------------------------------------
//
QVariant CProjectScriptWrapperReadOnly::scene(const QString& sName)
{
  QReadLocker locker(&m_spData->m_rwLock);
  for (qint32 iIndex = 0; m_spData->m_pluginData.m_vspScenes.size() > static_cast<size_t>(iIndex); ++iIndex)
  {
    QReadLocker sceneLocker(&m_spData->m_pluginData.m_vspScenes[static_cast<size_t>(iIndex)]->m_rwLock);
    if (m_spData->m_pluginData.m_vspScenes[static_cast<size_t>(iIndex)]->m_sName == sName)
    {
      CSceneScriptWrapperReadOnly* pScene = CreateSceneWrapper(m_spData->m_pluginData.m_vspScenes[static_cast<size_t>(iIndex)]);
      return CreateScriptObject(pScene, m_pEngine);
    }
  }
  for (qint32 iIndex = 0; m_spData->m_baseData.m_vspScenes.size() > static_cast<size_t>(iIndex); ++iIndex)
  {
    QReadLocker sceneLocker(&m_spData->m_baseData.m_vspScenes[static_cast<size_t>(iIndex)]->m_rwLock);
    if (m_spData->m_baseData.m_vspScenes[static_cast<size_t>(iIndex)]->m_sName == sName)
    {
      CSceneScriptWrapperReadOnly* pScene = CreateSceneWrapper(m_spData->m_baseData.m_vspScenes[static_cast<size_t>(iIndex)]);
      return CreateScriptObject(pScene, m_pEngine);
    }
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
QVariant CProjectScriptWrapperReadOnly::scene(qint32 iIndex)
{
  QReadLocker locker(&m_spData->m_rwLock);
  if (0 <= iIndex && m_spData->m_pluginData.m_vspScenes.size() > static_cast<size_t>(iIndex))
  {
    QReadLocker sceneLocker(&m_spData->m_pluginData.m_vspScenes[static_cast<size_t>(iIndex)]->m_rwLock);
    QString sName = m_spData->m_pluginData.m_vspScenes[static_cast<size_t>(iIndex)]->m_sName;
    sceneLocker.unlock();

    CSceneScriptWrapperReadOnly* pScene = CreateSceneWrapper(m_spData->m_pluginData.m_vspScenes[static_cast<size_t>(iIndex)]);
    return CreateScriptObject(pScene, m_pEngine);
  }
  if (0 <= iIndex && m_spData->m_baseData.m_vspScenes.size() > static_cast<size_t>(iIndex))
  {
    QReadLocker sceneLocker(&m_spData->m_baseData.m_vspScenes[static_cast<size_t>(iIndex)]->m_rwLock);
    QString sName = m_spData->m_baseData.m_vspScenes[static_cast<size_t>(iIndex)]->m_sName;
    sceneLocker.unlock();

    CSceneScriptWrapperReadOnly* pScene = CreateSceneWrapper(m_spData->m_baseData.m_vspScenes[static_cast<size_t>(iIndex)]);
    return CreateScriptObject(pScene, m_pEngine);
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
qint32 CProjectScriptWrapperReadOnly::numResources()
{
  QReadLocker locker(&m_spData->m_rwLock);
  std::set<QString> vResources;
  for (const auto& [sName, _] : m_spData->m_pluginData.m_spResourcesMap)
  {
    vResources.insert(sName);
  }
  for (const auto& [sName, _] : m_spData->m_baseData.m_spResourcesMap)
  {
    vResources.insert(sName);
  }
  return static_cast<qint32>(vResources.size());
}

//----------------------------------------------------------------------------------------
//
QStringList CProjectScriptWrapperReadOnly::resources()
{
  QReadLocker locker(&m_spData->m_rwLock);
  QStringList ret;
  for (auto it = m_spData->m_pluginData.m_spResourcesMap.begin(); m_spData->m_pluginData.m_spResourcesMap.end() != it; ++it)
  {
    ret << it->first;
  }
  for (auto it = m_spData->m_baseData.m_spResourcesMap.begin(); m_spData->m_baseData.m_spResourcesMap.end() != it; ++it)
  {
    ret << it->first;
  }
  ret.removeDuplicates();
  return ret;
}

//----------------------------------------------------------------------------------------
//
QVariant CProjectScriptWrapperReadOnly::resource(const QString& sValue)
{
  QReadLocker locker(&m_spData->m_rwLock);
  auto it = m_spData->m_pluginData.m_spResourcesMap.find(sValue);
  if (m_spData->m_pluginData.m_spResourcesMap.end() != it)
  {
    CResourceScriptWrapperReadOnly* pResource = CreateReosurceWrapper(it->second);
    return CreateScriptObject(pResource, m_pEngine);
  }
  it = m_spData->m_baseData.m_spResourcesMap.find(sValue);
  if (m_spData->m_baseData.m_spResourcesMap.end() != it)
  {
    CResourceScriptWrapperReadOnly* pResource = CreateReosurceWrapper(it->second);
    return CreateScriptObject(pResource, m_pEngine);
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
QVariant CProjectScriptWrapperReadOnly::resource(qint32 iIndex)
{
  QReadLocker locker(&m_spData->m_rwLock);
  auto it = m_spData->m_pluginData.m_spResourcesMap.begin();
  if (0 <= iIndex && m_spData->m_pluginData.m_spResourcesMap.size() > static_cast<size_t>(iIndex))
  {
    std::advance(it, iIndex);
    if (m_spData->m_pluginData.m_spResourcesMap.end() != it)
    {
      CResourceScriptWrapperReadOnly* pResource = CreateReosurceWrapper(it->second);
      return CreateScriptObject(pResource, m_pEngine);
    }
  }
  it = m_spData->m_baseData.m_spResourcesMap.begin();
  if (0 <= iIndex && m_spData->m_baseData.m_spResourcesMap.size() > static_cast<size_t>(iIndex))
  {
    std::advance(it, iIndex);
    if (m_spData->m_baseData.m_spResourcesMap.end() != it)
    {
      CResourceScriptWrapperReadOnly* pResource = CreateReosurceWrapper(it->second);
      return CreateScriptObject(pResource, m_pEngine);
    }
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
qint32 CProjectScriptWrapperReadOnly::numTags()
{
  QReadLocker locker(&m_spData->m_rwLock);
  std::set<QString> vTags;
  for (const auto& [sName, _] : m_spData->m_pluginData.m_vspTags)
  {
    vTags.insert(sName);
  }
  for (const auto& [sName, _] : m_spData->m_baseData.m_vspTags)
  {
    vTags.insert(sName);
  }
  return static_cast<qint32>(vTags.size());
}

//----------------------------------------------------------------------------------------
//
QStringList CProjectScriptWrapperReadOnly::tags()
{
  QReadLocker locker(&m_spData->m_rwLock);
  QStringList ret;
  for (auto it = m_spData->m_pluginData.m_vspTags.begin(); m_spData->m_pluginData.m_vspTags.end() != it; ++it)
  {
    ret << it->second->m_sName;
  }
  for (auto it = m_spData->m_baseData.m_vspTags.begin(); m_spData->m_baseData.m_vspTags.end() != it; ++it)
  {
    ret << it->second->m_sName;
  }
  ret.removeDuplicates();
  return ret;
}

//----------------------------------------------------------------------------------------
//
QVariant CProjectScriptWrapperReadOnly::tag(const QString& sValue)
{
  QReadLocker locker(&m_spData->m_rwLock);
  auto it = m_spData->m_pluginData.m_vspTags.find(sValue);
  if (m_spData->m_pluginData.m_vspTags.end() != it)
  {
    CTagWrapperReadOnly* pTag =
        new CTagWrapperReadOnly(m_pEngine, std::make_shared<STag>(*it->second));
    return CreateScriptObject(pTag, m_pEngine);
  }
  it = m_spData->m_baseData.m_vspTags.find(sValue);
  if (m_spData->m_baseData.m_vspTags.end() != it)
  {
    CTagWrapperReadOnly* pTag =
        new CTagWrapperReadOnly(m_pEngine, std::make_shared<STag>(*it->second));
    return CreateScriptObject(pTag, m_pEngine);
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
QVariant CProjectScriptWrapperReadOnly::tag(qint32 iIndex)
{
  QReadLocker locker(&m_spData->m_rwLock);
  auto it = m_spData->m_pluginData.m_vspTags.begin();
  if (0 <= iIndex && m_spData->m_pluginData.m_vspTags.size() > static_cast<size_t>(iIndex))
  {
    std::advance(it, iIndex);
    if (m_spData->m_pluginData.m_vspTags.end() != it)
    {
      CTagWrapperReadOnly* pTag =
          new CTagWrapperReadOnly(m_pEngine, std::make_shared<STag>(*it->second));
      return CreateScriptObject(pTag, m_pEngine);
    }
  }
  it = m_spData->m_baseData.m_vspTags.begin();
  if (0 <= iIndex && m_spData->m_baseData.m_vspTags.size() > static_cast<size_t>(iIndex))
  {
    std::advance(it, iIndex);
    if (m_spData->m_baseData.m_vspTags.end() != it)
    {
      CTagWrapperReadOnly* pTag =
          new CTagWrapperReadOnly(m_pEngine, std::make_shared<STag>(*it->second));
      return CreateScriptObject(pTag, m_pEngine);
    }
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
qint32 CProjectScriptWrapperReadOnly::numAchievements()
{
  QReadLocker locker(&m_spData->m_rwLock);
  std::set<QString> vAchs;
  for (const auto& [sName, _] : m_spData->m_pluginData.m_vspAchievements)
  {
    vAchs.insert(sName);
  }
  for (const auto& [sName, _] : m_spData->m_baseData.m_vspAchievements)
  {
    vAchs.insert(sName);
  }
  return static_cast<qint32>(vAchs.size());
}

//----------------------------------------------------------------------------------------
//
QStringList CProjectScriptWrapperReadOnly::achievements()
{
  QReadLocker locker(&m_spData->m_rwLock);
  QStringList ret;
  for (auto it = m_spData->m_pluginData.m_vspAchievements.begin(); m_spData->m_pluginData.m_vspAchievements.end() != it; ++it)
  {
    ret << it->second->m_sName;
  }
  for (auto it = m_spData->m_baseData.m_vspAchievements.begin(); m_spData->m_baseData.m_vspAchievements.end() != it; ++it)
  {
    ret << it->second->m_sName;
  }
  ret.removeDuplicates();
  return ret;
}

//----------------------------------------------------------------------------------------
//
QVariant CProjectScriptWrapperReadOnly::achievement(const QString& sValue)
{
  QReadLocker locker(&m_spData->m_rwLock);
  auto it = m_spData->m_pluginData.m_vspAchievements.find(sValue);
  if (m_spData->m_pluginData.m_vspAchievements.end() != it)
  {
    CSaveDataWrapperReadOnly* pTag = CreateSaveDataWrapper(it->second);
    return CreateScriptObject(pTag, m_pEngine);
  }
  it = m_spData->m_baseData.m_vspAchievements.find(sValue);
  if (m_spData->m_baseData.m_vspAchievements.end() != it)
  {
    CSaveDataWrapperReadOnly* pTag = CreateSaveDataWrapper(it->second);
    return CreateScriptObject(pTag, m_pEngine);
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
QVariant CProjectScriptWrapperReadOnly::achievement(qint32 iIndex)
{
  QReadLocker locker(&m_spData->m_rwLock);
  auto it = m_spData->m_pluginData.m_vspAchievements.begin();
  if (0 <= iIndex && m_spData->m_pluginData.m_vspAchievements.size() > static_cast<size_t>(iIndex))
  {
    std::advance(it, iIndex);
    if (m_spData->m_pluginData.m_vspAchievements.end() != it)
    {
      CSaveDataWrapperReadOnly* pTag = CreateSaveDataWrapper(it->second);
      return CreateScriptObject(pTag, m_pEngine);
    }
  }
  it = m_spData->m_baseData.m_vspAchievements.begin();
  if (0 <= iIndex && m_spData->m_baseData.m_vspAchievements.size() > static_cast<size_t>(iIndex))
  {
    std::advance(it, iIndex);
    if (m_spData->m_baseData.m_vspAchievements.end() != it)
    {
      CSaveDataWrapperReadOnly* pTag = CreateSaveDataWrapper(it->second);
      return CreateScriptObject(pTag, m_pEngine);
    }
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
CResourceScriptWrapperReadOnly* CProjectScriptWrapperReadOnly::
    CreateReosurceWrapper(const std::shared_ptr<SResource>& spResource) const
{
  return new CResourceScriptWrapperReadOnly(m_pEngine, spResource);
}

//----------------------------------------------------------------------------------------
//
CSaveDataWrapperReadOnly* CProjectScriptWrapperReadOnly::
    CreateSaveDataWrapper(const std::shared_ptr<SSaveData>& spSaveData) const
{
  return new CSaveDataWrapperReadOnly(m_pEngine, spSaveData);
}

//----------------------------------------------------------------------------------------
//
CSceneScriptWrapperReadOnly* CProjectScriptWrapperReadOnly::
    CreateSceneWrapper(const std::shared_ptr<SScene>& spScene) const
{
  return new CSceneScriptWrapperReadOnly(m_pEngine, spScene);
}

//----------------------------------------------------------------------------------------
//
CProjectScriptWrapperReadWrite::CProjectScriptWrapperReadWrite(tEngineType pEngine, const std::shared_ptr<SProject>& spProject) :
  CProjectScriptWrapperReadOnly(pEngine, spProject)
{
}
CProjectScriptWrapperReadWrite::~CProjectScriptWrapperReadWrite() = default;

//----------------------------------------------------------------------------------------
//
void CProjectScriptWrapperReadWrite::setVersion(qint32 iValue)
{
  QWriteLocker locker(&m_spData->m_rwLock);
  m_spData->m_iVersion = SVersion(iValue);
}

//----------------------------------------------------------------------------------------
//
void CProjectScriptWrapperReadWrite::setVersionText(const QString& sValue)
{
  QWriteLocker locker(&m_spData->m_rwLock);
  m_spData->m_iVersion = SVersion(sValue);
}

//----------------------------------------------------------------------------------------
//
void CProjectScriptWrapperReadWrite::setTargetVersion(qint32 iValue)
{
  QWriteLocker locker(&m_spData->m_rwLock);
  m_spData->m_iTargetVersion = SVersion(iValue);
}

//----------------------------------------------------------------------------------------
//
void CProjectScriptWrapperReadWrite::setTargetVersionText(const QString& sValue)
{
  QWriteLocker locker(&m_spData->m_rwLock);
  m_spData->m_iTargetVersion = SVersion(sValue);
}

//----------------------------------------------------------------------------------------
//
void CProjectScriptWrapperReadWrite::setDescribtion(const QString& sValue)
{
  QWriteLocker locker(&m_spData->m_rwLock);
  m_spData->m_sDescribtion = sValue;
}

//----------------------------------------------------------------------------------------
//
void CProjectScriptWrapperReadWrite::setTitleCard(const QString& sValue)
{
  QWriteLocker locker(&m_spData->m_rwLock);
  m_spData->m_sTitleCard = sValue;
}

//----------------------------------------------------------------------------------------
//
void CProjectScriptWrapperReadWrite::setMap(const QString& sValue)
{
  QWriteLocker locker(&m_spData->m_rwLock);
  m_spData->m_sMap = sValue;
}

//----------------------------------------------------------------------------------------
//
void CProjectScriptWrapperReadWrite::setSceneModel(const QString& sValue)
{
  QWriteLocker locker(&m_spData->m_rwLock);
  m_spData->m_sSceneModel = sValue;
}

//----------------------------------------------------------------------------------------
//
void CProjectScriptWrapperReadWrite::setPlayerLayout(const QString& sValue)
{
  QWriteLocker locker(&m_spData->m_rwLock);
  m_spData->m_sPlayerLayout = sValue;
}

//----------------------------------------------------------------------------------------
//
void CProjectScriptWrapperReadWrite::setNumberOfSoundEmitters(qint32 iValue)
{
  QWriteLocker locker(&m_spData->m_rwLock);
  m_spData->m_iNumberOfSoundEmitters = iValue;
}

//----------------------------------------------------------------------------------------
//
void CProjectScriptWrapperReadWrite::setMetCmdMode(qint32 iValue)
{
  QWriteLocker locker(&m_spData->m_rwLock);
  m_spData->m_metCmdMode = iValue;
}

//----------------------------------------------------------------------------------------
//
void CProjectScriptWrapperReadWrite::setFont(const QString& sValue)
{
  QWriteLocker locker(&m_spData->m_rwLock);
  m_spData->m_sFont = sValue;
}

//----------------------------------------------------------------------------------------
//
void CProjectScriptWrapperReadWrite::setUserData(const QString& sValue)
{
  QWriteLocker locker(&m_spData->m_rwLock);
  m_spData->m_sUserData = sValue;
}

//----------------------------------------------------------------------------------------
//
CResourceScriptWrapperReadOnly* CProjectScriptWrapperReadWrite::CreateReosurceWrapper(const std::shared_ptr<SResource>& spResource) const
{
  return new CResourceScriptWrapperReadWrite(m_pEngine, spResource);
}

//----------------------------------------------------------------------------------------
//
CSaveDataWrapperReadOnly* CProjectScriptWrapperReadWrite::CreateSaveDataWrapper(const std::shared_ptr<SSaveData>& spSaveData) const
{
  return new CSaveDataWrapperReadWrite(m_pEngine, spSaveData);
}

//----------------------------------------------------------------------------------------
//
CSceneScriptWrapperReadOnly* CProjectScriptWrapperReadWrite::CreateSceneWrapper(const std::shared_ptr<SScene>& spScene) const
{
  return new CSceneScriptWrapperReadWrite(m_pEngine, spScene);
}

//----------------------------------------------------------------------------------------
//
CResourceScriptWrapperReadOnly::CResourceScriptWrapperReadOnly(tEngineType pEngine, const std::shared_ptr<SResource>& spResource) :
  QObject(),
  CLockable(&spResource->m_rwLock),
  m_spData(spResource),
  m_pEngine(pEngine)
{
  assert(nullptr != spResource);
  assert(CheckEngineNotNull(pEngine));
}

CResourceScriptWrapperReadOnly::~CResourceScriptWrapperReadOnly()
{
}

//----------------------------------------------------------------------------------------
//
bool CResourceScriptWrapperReadOnly::isAnimatedImpl()
{
  QReadLocker locker(&m_spData->m_rwLock);
  switch (m_spData->m_type)
  {
    case EResourceType::eImage:
    {
      if (m_spData->m_sPath.IsLocalFile())
      {
        locker.unlock();
        QString sPath = m_spData->ResourceToAbsolutePath();
        if (QFileInfo::exists(sPath))
        {
          QImageReader reader(sPath);
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
bool CResourceScriptWrapperReadOnly::isLocalPath()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sPath.IsLocalFile();
}

//----------------------------------------------------------------------------------------
//
QString CResourceScriptWrapperReadOnly::getName()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sName;
}

//----------------------------------------------------------------------------------------
//
QUrl CResourceScriptWrapperReadOnly::getPath()
{
  if (nullptr == m_spData->m_spParent)
  {
    return static_cast<QUrl>(m_spData->m_sPath);
  }

  if (m_spData->m_sPath.IsLocalFile())
  {
    const QString sTruePathName = m_spData->ResourceToAbsolutePath("qrc:");
    return QUrl(sTruePathName);
  }
  else
  {
    return static_cast<QUrl>(m_spData->m_sPath);
  }
}

//----------------------------------------------------------------------------------------
//
QUrl CResourceScriptWrapperReadOnly::getSource()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sSource;
}

//----------------------------------------------------------------------------------------
//
CResourceScriptWrapperReadOnly::ResourceType CResourceScriptWrapperReadOnly::getType()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return ResourceType(m_spData->m_type._to_integral());
}

//----------------------------------------------------------------------------------------
//
QString CResourceScriptWrapperReadOnly::getResourceBundle()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sResourceBundle;
}

//----------------------------------------------------------------------------------------
//
bool CResourceScriptWrapperReadOnly::load()
{
  QReadLocker locker(&m_spData->m_rwLock);
  QString sResourceBundle = m_spData->m_sResourceBundle;
  if (!m_spData->m_sResourceBundle.isEmpty())
  {
    return CDatabaseManager::LoadBundle(m_spData->m_spParent, sResourceBundle);
  }
  return true;
}

//----------------------------------------------------------------------------------------
//
QVariant CResourceScriptWrapperReadOnly::project()
{
  QReadLocker locker(&m_spData->m_rwLock);
  if (nullptr != m_spData->m_spParent)
  {
    CProjectScriptWrapperReadOnly* pProject =
        new CProjectScriptWrapperReadOnly(m_pEngine, std::make_shared<SProject>(*m_spData->m_spParent));
    return CreateScriptObject(pProject, m_pEngine);
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
qint32 CResourceScriptWrapperReadOnly::numTags()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return static_cast<qint32>(m_spData->m_vsResourceTags.size());
}

//----------------------------------------------------------------------------------------
//
QStringList CResourceScriptWrapperReadOnly::tags()
{
  QReadLocker locker(&m_spData->m_rwLock);
  QStringList ret;
  for (auto it = m_spData->m_vsResourceTags.begin(); m_spData->m_vsResourceTags.end() != it; ++it)
  {
    ret << *it;
  }
  return ret;
}

//----------------------------------------------------------------------------------------
//
QVariant CResourceScriptWrapperReadOnly::tag(const QString& sValue)
{
  QReadLocker projLocker(&m_spData->m_spParent->m_rwLock);
  QReadLocker locker(&m_spData->m_rwLock);
  auto it = m_spData->m_vsResourceTags.find(sValue);
  if (m_spData->m_vsResourceTags.end() != it)
  {
    auto itTag = m_spData->m_spParent->m_pluginData.m_vspTags.find(*it);
    if (m_spData->m_spParent->m_pluginData.m_vspTags.end() != itTag)
    {
      CTagWrapperReadOnly* pTag =
          new CTagWrapperReadOnly(m_pEngine, std::make_shared<STag>(*itTag->second));
      return CreateScriptObject(pTag, m_pEngine);
    }
    itTag = m_spData->m_spParent->m_baseData.m_vspTags.find(*it);
    if (m_spData->m_spParent->m_baseData.m_vspTags.end() != itTag)
    {
      CTagWrapperReadOnly* pTag =
          new CTagWrapperReadOnly(m_pEngine, std::make_shared<STag>(*itTag->second));
      return CreateScriptObject(pTag, m_pEngine);
    }
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
QVariant CResourceScriptWrapperReadOnly::tag(qint32 iIndex)
{
  QReadLocker projLocker(&m_spData->m_spParent->m_rwLock);
  QReadLocker locker(&m_spData->m_rwLock);
  auto it = m_spData->m_vsResourceTags.begin();
  if (0 <= iIndex && m_spData->m_vsResourceTags.size() > static_cast<size_t>(iIndex))
  {
    std::advance(it, iIndex);
    if (m_spData->m_vsResourceTags.end() != it)
    {
      auto itTag = m_spData->m_spParent->m_pluginData.m_vspTags.find(*it);
      if (m_spData->m_spParent->m_pluginData.m_vspTags.end() != itTag)
      {
      CTagWrapperReadOnly* pTag =
          new CTagWrapperReadOnly(m_pEngine, std::make_shared<STag>(*itTag->second));
      return CreateScriptObject(pTag, m_pEngine);
      }
      itTag = m_spData->m_spParent->m_baseData.m_vspTags.find(*it);
      if (m_spData->m_spParent->m_baseData.m_vspTags.end() != itTag)
      {
        CTagWrapperReadOnly* pTag =
          new CTagWrapperReadOnly(m_pEngine, std::make_shared<STag>(*itTag->second));
        return CreateScriptObject(pTag, m_pEngine);
      }
    }
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
CProjectScriptWrapperReadOnly* CResourceScriptWrapperReadOnly::
    CreateProjectWrapper(const std::shared_ptr<SProject>& spProject) const
{
  return new CProjectScriptWrapperReadOnly(m_pEngine, std::make_shared<SProject>(*spProject));
}

//----------------------------------------------------------------------------------------
//
CResourceScriptWrapperReadWrite::CResourceScriptWrapperReadWrite(tEngineType pEngine, const std::shared_ptr<SResource>& spResource) :
  CResourceScriptWrapperReadOnly(pEngine, spResource)
{
}
CResourceScriptWrapperReadWrite::~CResourceScriptWrapperReadWrite() = default;

//----------------------------------------------------------------------------------------
//
void CResourceScriptWrapperReadWrite::addTag(const QString& sName, const QString& sCategory, const QString& sDescription)
{
  QWriteLocker projLocker(&m_spData->m_spParent->m_rwLock);
  QWriteLocker locker(&m_spData->m_rwLock);
  m_spData->m_vsResourceTags.insert(sName);
  auto itTagP = m_spData->m_spParent->m_pluginData.m_vspTags.find(sName);
  auto itTagB = m_spData->m_spParent->m_baseData.m_vspTags.find(sName);
  if (m_spData->m_spParent->m_pluginData.m_vspTags.end() != itTagP &&
      m_spData->m_spParent->m_baseData.m_vspTags.end() != itTagB)
  {
    m_spData->m_spParent->m_baseData.m_vspTags.insert({sName, std::make_shared<STag>(sName, sCategory, sDescription) });
  }
}

//----------------------------------------------------------------------------------------
//
void CResourceScriptWrapperReadWrite::removeTag(const QString& sValue)
{
  QWriteLocker projLocker(&m_spData->m_spParent->m_rwLock);
  QWriteLocker locker(&m_spData->m_rwLock);
  auto it = m_spData->m_vsResourceTags.find(sValue);
  if (m_spData->m_vsResourceTags.end() != it)
  {
    m_spData->m_vsResourceTags.erase(it);
  }
}

//----------------------------------------------------------------------------------------
//
void CResourceScriptWrapperReadWrite::removeTag(qint32 iIndex)
{
  QWriteLocker projLocker(&m_spData->m_spParent->m_rwLock);
  QWriteLocker locker(&m_spData->m_rwLock);
  auto it = m_spData->m_vsResourceTags.begin();
  if (0 <= iIndex && m_spData->m_vsResourceTags.size() > static_cast<size_t>(iIndex))
  {
    std::advance(it, iIndex);
    m_spData->m_vsResourceTags.erase(it);
  }
}

//----------------------------------------------------------------------------------------
//
CProjectScriptWrapperReadOnly* CResourceScriptWrapperReadWrite::
    CreateProjectWrapper(const std::shared_ptr<SProject>& spProject) const
{
  return new CProjectScriptWrapperReadWrite(m_pEngine, spProject);
}

//----------------------------------------------------------------------------------------
//
CSceneScriptWrapperReadOnly::CSceneScriptWrapperReadOnly(tEngineType pEngine, const std::shared_ptr<SScene>& spScene) :
  QObject(),
  CLockable(&spScene->m_rwLock),
  m_spData(spScene),
  m_pEngine(pEngine)
{
  assert(nullptr != spScene);
  assert(CheckEngineNotNull(pEngine));
}

CSceneScriptWrapperReadOnly::~CSceneScriptWrapperReadOnly()
{
}

//----------------------------------------------------------------------------------------
//
qint32 CSceneScriptWrapperReadOnly::getId()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_iId;
}

//----------------------------------------------------------------------------------------
//
QString CSceneScriptWrapperReadOnly::getName()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sName;
}

//----------------------------------------------------------------------------------------
//
QString CSceneScriptWrapperReadOnly::getScript()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sScript;
}

//----------------------------------------------------------------------------------------
//
QString CSceneScriptWrapperReadOnly::getSceneLayout()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sSceneLayout;
}

//----------------------------------------------------------------------------------------
//
bool CSceneScriptWrapperReadOnly::getCanStartHere()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_bCanStartHere;
}

//----------------------------------------------------------------------------------------
//
QString CSceneScriptWrapperReadOnly::getTitleCard()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sTitleCard;
}

//----------------------------------------------------------------------------------------
//
qint32 CSceneScriptWrapperReadOnly::numResources()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return static_cast<qint32>(m_spData->m_vsResourceRefs.size());
}

//----------------------------------------------------------------------------------------
//
QStringList CSceneScriptWrapperReadOnly::resources()
{
  QReadLocker locker(&m_spData->m_rwLock);
  QStringList ret;
  for (auto it = m_spData->m_vsResourceRefs.begin(); m_spData->m_vsResourceRefs.end() != it; ++it)
  {
    ret << *it;
  }
  return ret;
}

//----------------------------------------------------------------------------------------
//
QVariant CSceneScriptWrapperReadOnly::resource(const QString& sValue)
{
  QReadLocker locker(&m_spData->m_rwLock);
  if (nullptr != m_spData->m_spParent)
  {
    auto it = m_spData->m_vsResourceRefs.find(sValue);
    if (it != m_spData->m_vsResourceRefs.end())
    {
      QReadLocker locker(&m_spData->m_spParent->m_rwLock);
      auto itRef = m_spData->m_spParent->m_pluginData.m_spResourcesMap.find(sValue);
      if (m_spData->m_spParent->m_pluginData.m_spResourcesMap.end() != itRef)
      {
        locker.unlock();

        CResourceScriptWrapperReadOnly* pResource = CreateReosurceWrapper(itRef->second);
        return CreateScriptObject(pResource, m_pEngine);
      }
      itRef = m_spData->m_spParent->m_baseData.m_spResourcesMap.find(sValue);
      if (m_spData->m_spParent->m_baseData.m_spResourcesMap.end() != itRef)
      {
        locker.unlock();

        CResourceScriptWrapperReadOnly* pResource = CreateReosurceWrapper(itRef->second);
        return CreateScriptObject(pResource, m_pEngine);
      }
      return QVariant();
    }
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
QVariant CSceneScriptWrapperReadOnly::project()
{
  QReadLocker locker(&m_spData->m_rwLock);
  if (nullptr != m_spData->m_spParent)
  {
    CProjectScriptWrapperReadOnly* pProject = CreateProjectWrapper(m_spData->m_spParent);
    return CreateScriptObject(pProject, m_pEngine);
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
CProjectScriptWrapperReadOnly* CSceneScriptWrapperReadOnly::
    CreateProjectWrapper(const std::shared_ptr<SProject>& spProject) const
{
  return new CProjectScriptWrapperReadOnly(m_pEngine, spProject);
}

//----------------------------------------------------------------------------------------
//
CResourceScriptWrapperReadOnly* CSceneScriptWrapperReadOnly::
    CreateReosurceWrapper(const std::shared_ptr<SResource>& spResource) const
{
  return new CResourceScriptWrapperReadOnly(m_pEngine, spResource);
}

//----------------------------------------------------------------------------------------
//
CSceneScriptWrapperReadWrite::CSceneScriptWrapperReadWrite(tEngineType pEngine, const std::shared_ptr<SScene>& spScene) :
  CSceneScriptWrapperReadOnly(pEngine, spScene)
{
}
CSceneScriptWrapperReadWrite::~CSceneScriptWrapperReadWrite() = default;

//----------------------------------------------------------------------------------------
//
void CSceneScriptWrapperReadWrite::setScript(const QString& sValue)
{
  QWriteLocker locker(&m_spData->m_rwLock);
  m_spData->m_sScript = sValue;
}

//----------------------------------------------------------------------------------------
//
void CSceneScriptWrapperReadWrite::setSceneLayout(const QString& sValue)
{
  QWriteLocker locker(&m_spData->m_rwLock);
  m_spData->m_sSceneLayout = sValue;
}

//----------------------------------------------------------------------------------------
//
void CSceneScriptWrapperReadWrite::setTitleCard(const QString& sValue)
{
  QWriteLocker locker(&m_spData->m_rwLock);
  m_spData->m_sTitleCard = sValue;
}

//----------------------------------------------------------------------------------------
//
void CSceneScriptWrapperReadWrite::addResource(const QString& sName)
{
  QWriteLocker locker(&m_spData->m_rwLock);
  if (nullptr != m_spData->m_spParent)
  {
    QWriteLocker locker(&m_spData->m_spParent->m_rwLock);
    auto itRef = m_spData->m_spParent->m_pluginData.m_spResourcesMap.find(sName);
    if (m_spData->m_spParent->m_pluginData.m_spResourcesMap.end() != itRef)
    {
      m_spData->m_vsResourceRefs.insert(sName);
    }
    itRef = m_spData->m_spParent->m_baseData.m_spResourcesMap.find(sName);
    if (m_spData->m_spParent->m_baseData.m_spResourcesMap.end() != itRef)
    {
      m_spData->m_vsResourceRefs.insert(sName);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneScriptWrapperReadWrite::removeResource(const QString& sValue)
{
  QWriteLocker locker(&m_spData->m_rwLock);
  auto it = m_spData->m_vsResourceRefs.find(sValue);
  if (it != m_spData->m_vsResourceRefs.end())
  {
    m_spData->m_vsResourceRefs.erase(it);
  }
}

//----------------------------------------------------------------------------------------
//
CProjectScriptWrapperReadOnly* CSceneScriptWrapperReadWrite::CreateProjectWrapper(const std::shared_ptr<SProject>& spProject) const
{
  return new CProjectScriptWrapperReadWrite(m_pEngine, spProject);
}

//----------------------------------------------------------------------------------------
//
CResourceScriptWrapperReadOnly* CSceneScriptWrapperReadWrite::CreateReosurceWrapper(const std::shared_ptr<SResource>& spResource) const
{
  return new CResourceScriptWrapperReadWrite(m_pEngine, spResource);
}

//----------------------------------------------------------------------------------------
//
CKinkWrapperReadOnly::CKinkWrapperReadOnly(tEngineType pEngine, const std::shared_ptr<SKink>& spKink) :
  QObject(),
  m_spData(spKink),
  m_pEngine(pEngine)
{
  assert(nullptr != spKink);
  assert(CheckEngineNotNull(pEngine));
}

CKinkWrapperReadOnly::~CKinkWrapperReadOnly()
{

}

//----------------------------------------------------------------------------------------
//
qint32 CKinkWrapperReadOnly::getIdForOrdering()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return static_cast<qint32>(m_spData->m_iIdForOrdering);
}

//----------------------------------------------------------------------------------------
//
QString CKinkWrapperReadOnly::getType()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sType;
}

//----------------------------------------------------------------------------------------
//
QString CKinkWrapperReadOnly::getName()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sName;
}

//----------------------------------------------------------------------------------------
//
QString CKinkWrapperReadOnly::getDescribtion()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sDescribtion;
}

//----------------------------------------------------------------------------------------
//
QColor CKinkWrapperReadOnly::color()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return CalculateTagColor(*m_spData);
}

//----------------------------------------------------------------------------------------
//
CTagWrapperReadOnly::CTagWrapperReadOnly(tEngineType pEngine, const std::shared_ptr<STag>& spTag) :
    QObject(),
    m_spData(spTag),
    m_pEngine(pEngine)
{
  assert(nullptr != spTag);
  assert(CheckEngineNotNull(pEngine));
}
CTagWrapperReadOnly::~CTagWrapperReadOnly()
{
}

//----------------------------------------------------------------------------------------
//
QString CTagWrapperReadOnly::getType()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sType;
}

//----------------------------------------------------------------------------------------
//
QString CTagWrapperReadOnly::getName()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sName;
}

//----------------------------------------------------------------------------------------
//
QString CTagWrapperReadOnly::getDescribtion()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sDescribtion;
}

//----------------------------------------------------------------------------------------
//
QColor CTagWrapperReadOnly::color()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return CalculateTagColor(*m_spData);
}

//----------------------------------------------------------------------------------------
//
qint32 CTagWrapperReadOnly::numResources()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return static_cast<qint32>(m_spData->m_vsResourceRefs.size());
}

//----------------------------------------------------------------------------------------
//
QStringList CTagWrapperReadOnly::resources()
{
  QReadLocker locker(&m_spData->m_rwLock);
  QStringList vsRet;
  for (const QString& sRes : m_spData->m_vsResourceRefs)
  {
    vsRet << sRes;
  }
  return vsRet;
}

//----------------------------------------------------------------------------------------
//
CDialogueWrapperBaseReadOnly::CDialogueWrapperBaseReadOnly(tEngineType pEngine, const std::shared_ptr<CDialogueNode>& spData) :
  m_pEngine(pEngine),
  m_spData(spData)
{
}
CDialogueWrapperBaseReadOnly::~CDialogueWrapperBaseReadOnly()
{

}

//----------------------------------------------------------------------------------------
//
QString CDialogueWrapperBaseReadOnly::getResource() const
{
  return m_spData->m_sFileId;
}

//----------------------------------------------------------------------------------------
//
QString CDialogueWrapperBaseReadOnly::getName() const
{
  return m_spData->m_sName;
}

//----------------------------------------------------------------------------------------
//
CDialogueWrapperReadOnly::CDialogueWrapperReadOnly(tEngineType pEngine, const std::shared_ptr<CDialogueNodeDialogue>& spData) :
  CDialogueWrapperBaseReadOnly(pEngine, spData),
  m_spData(spData)
{
}
CDialogueWrapperReadOnly::~CDialogueWrapperReadOnly()
{
}

//----------------------------------------------------------------------------------------
//
bool CDialogueWrapperReadOnly::getHasCondition() const
{
  return m_spData->m_bHasCondition;
}

//----------------------------------------------------------------------------------------
//
qint32 CDialogueWrapperReadOnly::numDialogueData()
{
  return static_cast<qint32>(m_spData->m_vspChildren.size());
}

//----------------------------------------------------------------------------------------
//
QStringList CDialogueWrapperReadOnly::dialogueDataList()
{
  QStringList ret;
  for (auto it = m_spData->m_vspChildren.begin(); m_spData->m_vspChildren.end() != it; ++it)
  {
    ret << (*it)->m_sName;
  }
  return ret;
}

//----------------------------------------------------------------------------------------
//
QVariant CDialogueWrapperReadOnly::dialogueData(const QString& sValue)
{
  auto it = std::find_if(m_spData->m_vspChildren.begin(), m_spData->m_vspChildren.end(),
                         [&](const std::shared_ptr<CDialogueNode>& spData) {
                           return sValue == spData->m_sName;
                         });
  if (m_spData->m_vspChildren.end() != it)
  {
    CDialogueDataWrapperReadOnly* pWrapper =
        new CDialogueDataWrapperReadOnly(m_pEngine, std::dynamic_pointer_cast<CDialogueData>(*it));
    return CreateScriptObject(pWrapper, m_pEngine);
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
QVariant CDialogueWrapperReadOnly::dialogueData(qint32 iIndex)
{
  if (0 > iIndex || static_cast<qint32>(m_spData->m_vspChildren.size()) <= iIndex)
  {
    return QVariant();
  }
  auto it = m_spData->m_vspChildren.begin();
  std::advance(it, iIndex);
  if (m_spData->m_vspChildren.end() != it)
  {
    CDialogueDataWrapperReadOnly* pWrapper =
        new CDialogueDataWrapperReadOnly(m_pEngine, std::dynamic_pointer_cast<CDialogueData>(*it));
    return CreateScriptObject(pWrapper, m_pEngine);
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
qint32 CDialogueWrapperReadOnly::numTags()
{
  return static_cast<qint32>(m_spData->m_tags.size());
}

//----------------------------------------------------------------------------------------
//
QStringList CDialogueWrapperReadOnly::tags()
{
  QStringList ret;
  for (auto it = m_spData->m_tags.begin(); m_spData->m_tags.end() != it; ++it)
  {
    ret << it->first;
  }
  return ret;
}

//----------------------------------------------------------------------------------------
//
QVariant CDialogueWrapperReadOnly::tag(const QString& sValue)
{
  auto it = m_spData->m_tags.find(sValue);
  if (m_spData->m_tags.end() != it)
  {
    CTagWrapperReadOnly* pTag =
        new CTagWrapperReadOnly(m_pEngine, std::make_shared<STag>(*it->second));
    return CreateScriptObject(pTag, m_pEngine);
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
QVariant CDialogueWrapperReadOnly::tag(qint32 iIndex)
{
  if (m_spData->m_tags.size() > static_cast<size_t>(iIndex) && 0 <= iIndex)
  {
    auto itTag = m_spData->m_tags.begin();
    std::advance(itTag, iIndex);
    CTagWrapperReadOnly* pTag =
        new CTagWrapperReadOnly(m_pEngine, std::make_shared<STag>(*itTag->second));
    return CreateScriptObject(pTag, m_pEngine);
  }
  return QVariant();
}


//----------------------------------------------------------------------------------------
//
CDialogueDataWrapperReadOnly::CDialogueDataWrapperReadOnly(tEngineType pEngine, const std::shared_ptr<CDialogueData>& spData) :
  CDialogueWrapperBaseReadOnly(pEngine, spData),
  m_spData(spData)
{
}
CDialogueDataWrapperReadOnly::~CDialogueDataWrapperReadOnly()
{
}

//----------------------------------------------------------------------------------------
//
QString CDialogueDataWrapperReadOnly::getCondition() const
{
  return m_spData->m_sCondition;
}

//----------------------------------------------------------------------------------------
//
QString CDialogueDataWrapperReadOnly::getString() const
{
  return m_spData->m_sString;
}

//----------------------------------------------------------------------------------------
//
QString CDialogueDataWrapperReadOnly::getSoundResource() const
{
  return m_spData->m_sSoundResource;
}

//----------------------------------------------------------------------------------------
//
qint64 CDialogueDataWrapperReadOnly::getWaitTime() const
{
  return m_spData->m_iWaitTimeMs;
}

//----------------------------------------------------------------------------------------
//
bool CDialogueDataWrapperReadOnly::getSkipable() const
{
  return m_spData->m_bSkipable;
}

//----------------------------------------------------------------------------------------
//
CSaveDataWrapperReadOnly::CSaveDataWrapperReadOnly(tEngineType pEngine, const std::shared_ptr<SSaveData>& spData) :
  m_spData(spData),
  m_pEngine(pEngine)
{
  assert(nullptr != m_spData);
  assert(CheckEngineNotNull(pEngine));
}
CSaveDataWrapperReadOnly::~CSaveDataWrapperReadOnly()
{}

//----------------------------------------------------------------------------------------
//
QString CSaveDataWrapperReadOnly::getName() const
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sName;
}

//----------------------------------------------------------------------------------------
//
QString CSaveDataWrapperReadOnly::getDescribtion() const
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sDescribtion;
}

//----------------------------------------------------------------------------------------
//
CSaveDataWrapperReadOnly::SaveDataType CSaveDataWrapperReadOnly::getType() const
{
  QReadLocker locker(&m_spData->m_rwLock);
  return CSaveDataWrapperReadOnly::SaveDataType(m_spData->m_type._to_integral());
}

//----------------------------------------------------------------------------------------
//
QString CSaveDataWrapperReadOnly::getResource() const
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sResource;
}

//----------------------------------------------------------------------------------------
//
QVariant CSaveDataWrapperReadOnly::getData() const
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_data;
}

//----------------------------------------------------------------------------------------
//
CSaveDataWrapperReadWrite::CSaveDataWrapperReadWrite(tEngineType pEngine, const std::shared_ptr<SSaveData>& spData) :
  CSaveDataWrapperReadOnly(pEngine, spData)
{
}
CSaveDataWrapperReadWrite::~CSaveDataWrapperReadWrite() = default;

//----------------------------------------------------------------------------------------
//
void CSaveDataWrapperReadWrite::setDescribtion(const QString& sValue) const
{
  QWriteLocker locker(&m_spData->m_rwLock);
  m_spData->m_sDescribtion = sValue;
}

//----------------------------------------------------------------------------------------
//
void CSaveDataWrapperReadWrite::setResource(const QString& sRes) const
{
  QWriteLocker locker(&m_spData->m_rwLock);
  m_spData->m_sResource = sRes;
}

//----------------------------------------------------------------------------------------
//
void CSaveDataWrapperReadWrite::setData(const QVariant& var) const
{
  QWriteLocker locker(&m_spData->m_rwLock);
  m_spData->m_data = var;
}
