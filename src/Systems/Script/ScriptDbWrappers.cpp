#include "ScriptDbWrappers.h"
#include "Application.h"
#include "Systems/Tag.h"

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
  bool CheckEngineNotNull(tEngineType pEngine)
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
CProjectScriptWrapper::CProjectScriptWrapper(tEngineType pEngine, const std::shared_ptr<SProject>& spProject) :
  QObject(),
  CLockable(&spProject->m_rwLock),
  m_spData(spProject),
  m_pEngine(pEngine)
{
  assert(nullptr != spProject);
  assert(CheckEngineNotNull(pEngine));
}

CProjectScriptWrapper::~CProjectScriptWrapper()
{
}

//----------------------------------------------------------------------------------------
//
qint32 CProjectScriptWrapper::getId()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_iId;
}

//----------------------------------------------------------------------------------------
//
qint32 CProjectScriptWrapper::getVersion()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return static_cast<qint32>(m_spData->m_iVersion);
}

//----------------------------------------------------------------------------------------
//
QString CProjectScriptWrapper::getVersionText()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return static_cast<QString>(m_spData->m_iVersion);
}

//----------------------------------------------------------------------------------------
//
qint32 CProjectScriptWrapper::getTargetVersion()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return static_cast<qint32>(m_spData->m_iTargetVersion);
}

//----------------------------------------------------------------------------------------
//
QString CProjectScriptWrapper::getTargetVersionText()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return static_cast<QString>(m_spData->m_iTargetVersion);
}

//----------------------------------------------------------------------------------------
//
QString CProjectScriptWrapper::getName()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sName;
}

//----------------------------------------------------------------------------------------
//
QString CProjectScriptWrapper::getFolderName()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sFolderName;
}

//----------------------------------------------------------------------------------------
//
QString CProjectScriptWrapper::getDescribtion()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sDescribtion;
}

//----------------------------------------------------------------------------------------
//
QString CProjectScriptWrapper::getTitleCard()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sTitleCard;
}

//----------------------------------------------------------------------------------------
//
QString CProjectScriptWrapper::getMap()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sMap;
}

//----------------------------------------------------------------------------------------
//
QString CProjectScriptWrapper::getSceneModel()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sSceneModel;
}

//----------------------------------------------------------------------------------------
//
QString CProjectScriptWrapper::getPlayerLayout()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sPlayerLayout;
}

//----------------------------------------------------------------------------------------
//
qint32 CProjectScriptWrapper::getNumberOfSoundEmitters()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_iNumberOfSoundEmitters;
}

//----------------------------------------------------------------------------------------
//
bool CProjectScriptWrapper::isUsingWeb()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_bUsesWeb;
}

//----------------------------------------------------------------------------------------
//
bool CProjectScriptWrapper::isUsingCodecs()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_bNeedsCodecs;
}

//----------------------------------------------------------------------------------------
//
bool CProjectScriptWrapper::isBundled()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_bBundled;
}

//----------------------------------------------------------------------------------------
//
bool CProjectScriptWrapper::isReadOnly()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_bReadOnly;
}

//----------------------------------------------------------------------------------------
//
bool CProjectScriptWrapper::isLoaded()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_bLoaded;
}

//----------------------------------------------------------------------------------------
//
CProjectScriptWrapper::DownLoadState CProjectScriptWrapper::getDlState()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return static_cast<DownLoadState>(m_spData->m_dlState._to_integral());
}

//----------------------------------------------------------------------------------------
//
QString CProjectScriptWrapper::getFont()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sFont;
}

//----------------------------------------------------------------------------------------
//
QString CProjectScriptWrapper::getUserData()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sUserData;
}

//----------------------------------------------------------------------------------------
//
qint32 CProjectScriptWrapper::numKinks()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return static_cast<qint32>(m_spData->m_vsKinks.size());
}

//----------------------------------------------------------------------------------------
//
QStringList CProjectScriptWrapper::kinks()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_vsKinks;
}

//----------------------------------------------------------------------------------------
//
QVariant CProjectScriptWrapper::kink(const QString& sName)
{
  QReadLocker locker(&m_spData->m_rwLock);
  auto it = std::find(m_spData->m_vsKinks.begin(), m_spData->m_vsKinks.end(), sName);
  if (m_spData->m_vsKinks.end() != it)
  {
    auto wpDbManager = CApplication::Instance()->System<CDatabaseManager>().lock();
    if (nullptr != wpDbManager)
    {
      tspKink spKink = wpDbManager->FindKink(sName);
      if (nullptr != spKink)
      {
        CKinkWrapper* pKink = new CKinkWrapper(m_pEngine, spKink);
        return CreateScriptObject(pKink, m_pEngine);
      }
    }
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
qint32 CProjectScriptWrapper::numScenes()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return static_cast<qint32>(m_spData->m_vspScenes.size());
}

//----------------------------------------------------------------------------------------
//
QStringList CProjectScriptWrapper::scenes()
{
  QStringList outList;
  QReadLocker locker(&m_spData->m_rwLock);
  for (qint32 iIndex = 0; m_spData->m_vspScenes.size() > static_cast<size_t>(iIndex); ++iIndex)
  {
    QReadLocker sceneLocker(&m_spData->m_vspScenes[static_cast<size_t>(iIndex)]->m_rwLock);
    outList << m_spData->m_vspScenes[static_cast<size_t>(iIndex)]->m_sName;
  }
  return outList;
}

//----------------------------------------------------------------------------------------
//
QVariant CProjectScriptWrapper::scene(const QString& sName)
{
  QReadLocker locker(&m_spData->m_rwLock);
  for (qint32 iIndex = 0; m_spData->m_vspScenes.size() > static_cast<size_t>(iIndex); ++iIndex)
  {
    QReadLocker sceneLocker(&m_spData->m_vspScenes[static_cast<size_t>(iIndex)]->m_rwLock);
    if (m_spData->m_vspScenes[static_cast<size_t>(iIndex)]->m_sName == sName)
    {
      CSceneScriptWrapper* pScene =
        new CSceneScriptWrapper(m_pEngine, std::make_shared<SScene>(*m_spData->m_vspScenes[static_cast<size_t>(iIndex)]));
      return CreateScriptObject(pScene, m_pEngine);
    }
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
QVariant CProjectScriptWrapper::scene(qint32 iIndex)
{
  QReadLocker locker(&m_spData->m_rwLock);
  if (0 <= iIndex && m_spData->m_vspScenes.size() > static_cast<size_t>(iIndex))
  {
    QReadLocker sceneLocker(&m_spData->m_vspScenes[static_cast<size_t>(iIndex)]->m_rwLock);
    QString sName = m_spData->m_vspScenes[static_cast<size_t>(iIndex)]->m_sName;
    sceneLocker.unlock();

    CSceneScriptWrapper* pScene =
      new CSceneScriptWrapper(m_pEngine, std::make_shared<SScene>(*m_spData->m_vspScenes[static_cast<size_t>(iIndex)]));
    return CreateScriptObject(pScene, m_pEngine);
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
qint32 CProjectScriptWrapper::numResources()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return static_cast<qint32>(m_spData->m_spResourcesMap.size());
}

//----------------------------------------------------------------------------------------
//
QStringList CProjectScriptWrapper::resources()
{
  QReadLocker locker(&m_spData->m_rwLock);
  QStringList ret;
  for (auto it = m_spData->m_spResourcesMap.begin(); m_spData->m_spResourcesMap.end() != it; ++it)
  {
    ret << it->first;
  }
  return ret;
}

//----------------------------------------------------------------------------------------
//
QVariant CProjectScriptWrapper::resource(const QString& sValue)
{
  QReadLocker locker(&m_spData->m_rwLock);
  auto it = m_spData->m_spResourcesMap.find(sValue);
  if (m_spData->m_spResourcesMap.end() != it)
  {
    CResourceScriptWrapper* pResource =
      new CResourceScriptWrapper(m_pEngine, std::make_shared<SResource>(*it->second));
    return CreateScriptObject(pResource, m_pEngine);
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
QVariant CProjectScriptWrapper::resource(qint32 iIndex)
{
  QReadLocker locker(&m_spData->m_rwLock);
  auto it = m_spData->m_spResourcesMap.begin();
  if (0 <= iIndex && m_spData->m_spResourcesMap.size() > static_cast<size_t>(iIndex))
  {
    std::advance(it, iIndex);
    if (m_spData->m_spResourcesMap.end() != it)
    {
      CResourceScriptWrapper* pResource =
        new CResourceScriptWrapper(m_pEngine, std::make_shared<SResource>(*it->second));
      return CreateScriptObject(pResource, m_pEngine);
    }
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
qint32 CProjectScriptWrapper::numTags()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return static_cast<qint32>(m_spData->m_vspTags.size());
}

//----------------------------------------------------------------------------------------
//
QStringList CProjectScriptWrapper::tags()
{
  QReadLocker locker(&m_spData->m_rwLock);
  QStringList ret;
  for (auto it = m_spData->m_vspTags.begin(); m_spData->m_vspTags.end() != it; ++it)
  {
    ret << it->second->m_sName;
  }
  return ret;
}

//----------------------------------------------------------------------------------------
//
QVariant CProjectScriptWrapper::tag(const QString& sValue)
{
  QReadLocker locker(&m_spData->m_rwLock);
  auto it = m_spData->m_vspTags.find(sValue);
  if (m_spData->m_vspTags.end() != it)
  {
    CTagWrapper* pTag =
        new CTagWrapper(m_pEngine, std::make_shared<STag>(*it->second));
    return CreateScriptObject(pTag, m_pEngine);
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
QVariant CProjectScriptWrapper::tag(qint32 iIndex)
{
  QReadLocker locker(&m_spData->m_rwLock);
  auto it = m_spData->m_vspTags.begin();
  if (0 <= iIndex && m_spData->m_vspTags.size() > static_cast<size_t>(iIndex))
  {
    std::advance(it, iIndex);
    if (m_spData->m_vspTags.end() != it)
    {
      CTagWrapper* pTag =
          new CTagWrapper(m_pEngine, std::make_shared<STag>(*it->second));
      return CreateScriptObject(pTag, m_pEngine);
    }
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
CResourceScriptWrapper::CResourceScriptWrapper(tEngineType pEngine, const std::shared_ptr<SResource>& spResource) :
  QObject(),
  CLockable(&spResource->m_rwLock),
  m_spData(spResource),
  m_pEngine(pEngine)
{
  assert(nullptr != spResource);
  assert(CheckEngineNotNull(pEngine));
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

  if (IsLocalFile(m_spData->m_sPath))
  {
    const QString sTruePathName = ResourceUrlToAbsolutePath(m_spData, "qrc:");
    return QUrl(sTruePathName);
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
QString CResourceScriptWrapper::getResourceBundle()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sResourceBundle;
}

//----------------------------------------------------------------------------------------
//
bool CResourceScriptWrapper::load()
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
QVariant CResourceScriptWrapper::project()
{
  QReadLocker locker(&m_spData->m_rwLock);
  if (nullptr != m_spData->m_spParent)
  {
    CProjectScriptWrapper* pProject =
        new CProjectScriptWrapper(m_pEngine, std::make_shared<SProject>(*m_spData->m_spParent));
    return CreateScriptObject(pProject, m_pEngine);
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
qint32 CResourceScriptWrapper::numTags()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return static_cast<qint32>(m_spData->m_vsResourceTags.size());
}

//----------------------------------------------------------------------------------------
//
QStringList CResourceScriptWrapper::tags()
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
QVariant CResourceScriptWrapper::tag(const QString& sValue)
{
  QReadLocker projLocker(&m_spData->m_spParent->m_rwLock);
  QReadLocker locker(&m_spData->m_rwLock);
  auto it = m_spData->m_vsResourceTags.find(sValue);
  if (m_spData->m_vsResourceTags.end() != it)
  {
    auto itTag = m_spData->m_spParent->m_vspTags.find(*it);
    if (m_spData->m_spParent->m_vspTags.end() != itTag)
    {
      CTagWrapper* pTag =
          new CTagWrapper(m_pEngine, std::make_shared<STag>(*itTag->second));
      return CreateScriptObject(pTag, m_pEngine);
    }
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
QVariant CResourceScriptWrapper::tag(qint32 iIndex)
{
  QReadLocker projLocker(&m_spData->m_spParent->m_rwLock);
  QReadLocker locker(&m_spData->m_rwLock);
  auto it = m_spData->m_vsResourceTags.begin();
  if (0 <= iIndex && m_spData->m_vsResourceTags.size() > static_cast<size_t>(iIndex))
  {
    std::advance(it, iIndex);
    if (m_spData->m_vsResourceTags.end() != it)
    {
      auto itTag = m_spData->m_spParent->m_vspTags.find(*it);
      if (m_spData->m_spParent->m_vspTags.end() != itTag)
      {
        CTagWrapper* pTag =
          new CTagWrapper(m_pEngine, std::make_shared<STag>(*itTag->second));
        return CreateScriptObject(pTag, m_pEngine);
      }
    }
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
CSceneScriptWrapper::CSceneScriptWrapper(tEngineType pEngine, const std::shared_ptr<SScene>& spScene) :
  QObject(),
  CLockable(&spScene->m_rwLock),
  m_spData(spScene),
  m_pEngine(pEngine)
{
  assert(nullptr != spScene);
  assert(CheckEngineNotNull(pEngine));
}

CSceneScriptWrapper::~CSceneScriptWrapper()
{
}

//----------------------------------------------------------------------------------------
//
qint32 CSceneScriptWrapper::getId()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_iId;
}

//----------------------------------------------------------------------------------------
//
QString CSceneScriptWrapper::getName()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sName;
}

//----------------------------------------------------------------------------------------
//
QString CSceneScriptWrapper::getScript()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sScript;
}

//----------------------------------------------------------------------------------------
//
QString CSceneScriptWrapper::getSceneLayout()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sSceneLayout;
}

//----------------------------------------------------------------------------------------
//
qint32 CSceneScriptWrapper::numResources()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return static_cast<qint32>(m_spData->m_vsResourceRefs.size());
}

//----------------------------------------------------------------------------------------
//
QStringList CSceneScriptWrapper::resources()
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
QVariant CSceneScriptWrapper::resource(const QString& sValue)
{
  QReadLocker locker(&m_spData->m_rwLock);
  if (nullptr != m_spData->m_spParent)
  {
    auto it = m_spData->m_vsResourceRefs.find(sValue);
    if (it != m_spData->m_vsResourceRefs.end())
    {
      QReadLocker locker(&m_spData->m_spParent->m_rwLock);
      auto itRef = m_spData->m_spParent->m_spResourcesMap.find(sValue);
      if (m_spData->m_spParent->m_spResourcesMap.end() != itRef)
      {
        locker.unlock();

        CResourceScriptWrapper* pResource =
            new CResourceScriptWrapper(m_pEngine, std::make_shared<SResource>(*itRef->second));
        return CreateScriptObject(pResource, m_pEngine);
      }
      return QVariant();
    }
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
QVariant CSceneScriptWrapper::project()
{
  QReadLocker locker(&m_spData->m_rwLock);
  if (nullptr != m_spData->m_spParent)
  {
    CProjectScriptWrapper* pProject =
        new CProjectScriptWrapper(m_pEngine, std::make_shared<SProject>(*m_spData->m_spParent));
    return CreateScriptObject(pProject, m_pEngine);
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
CKinkWrapper::CKinkWrapper(tEngineType pEngine, const std::shared_ptr<SKink>& spKink) :
  QObject(),
  m_spData(spKink),
  m_pEngine(pEngine)
{
  assert(nullptr != spKink);
  assert(CheckEngineNotNull(pEngine));
}

CKinkWrapper::~CKinkWrapper()
{

}

//----------------------------------------------------------------------------------------
//
qint32 CKinkWrapper::getIdForOrdering()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return static_cast<qint32>(m_spData->m_iIdForOrdering);
}

//----------------------------------------------------------------------------------------
//
QString CKinkWrapper::getType()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sType;
}

//----------------------------------------------------------------------------------------
//
QString CKinkWrapper::getName()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sName;
}

//----------------------------------------------------------------------------------------
//
QString CKinkWrapper::getDescribtion()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sDescribtion;
}

//----------------------------------------------------------------------------------------
//
QColor CKinkWrapper::color()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return CalculateTagColor(*m_spData);
}

//----------------------------------------------------------------------------------------
//
CTagWrapper::CTagWrapper(tEngineType pEngine, const std::shared_ptr<STag>& spTag) :
    QObject(),
    m_spData(spTag),
    m_pEngine(pEngine)
{
  assert(nullptr != spTag);
  assert(CheckEngineNotNull(pEngine));
}
CTagWrapper::~CTagWrapper()
{
}

//----------------------------------------------------------------------------------------
//
QString CTagWrapper::getType()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sType;
}

//----------------------------------------------------------------------------------------
//
QString CTagWrapper::getName()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sName;
}

//----------------------------------------------------------------------------------------
//
QString CTagWrapper::getDescribtion()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return m_spData->m_sDescribtion;
}

//----------------------------------------------------------------------------------------
//
QColor CTagWrapper::color()
{
  QReadLocker locker(&m_spData->m_rwLock);
  return CalculateTagColor(*m_spData);
}
