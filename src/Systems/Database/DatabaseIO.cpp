#include "DatabaseIO.h"
#include "Application.h"
#include "DatabaseData.h"
#include "Project.h"
#include "Resource.h"
#include "Settings.h"

#include "Systems/DatabaseManager.h"
#include "Systems/EOS/EosHelpers.h"
#include "Systems/PhysFs/PhysFsFileEngine.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QDirIterator>
#include <QDir>
#include <QFontDatabase>
#include <QJsonDocument>
#include <QResource>
#include <QTextStream>
#include <QtGlobal>

namespace  {
  const char c_sProjectBundleFileEnding[] = ".proj";
}

//----------------------------------------------------------------------------------------
//  Desktop Implementation
//----------------------------------------------------------------------------------------
#if defined(Q_OS_WIN) || defined(Q_OS_UNIX) || defined(Q_OS_ANDROID)
class CDatabaseIODefault : public CDatabaseIO
{
public:
  CDatabaseIODefault(CDatabaseManager* pManager, std::shared_ptr<CDatabaseData> spData) :
    CDatabaseIO(pManager, spData),
    m_spSettings(CApplication::Instance()->Settings())
  {
  }
  ~CDatabaseIODefault() override {}

protected:
  //----------------------------------------------------------------------------------------
  //
  QString ContentPath() const override
  {
    return m_spSettings->ContentFolder();
  }

  //----------------------------------------------------------------------------------------
  //
  bool DeserializeProjectImpl(tspProject& spProject) override
  {
    return DeserializeProjectImplStatic(spProject, false, [this](){ return m_pManager->FindNewProjectId(); });
  }

  //----------------------------------------------------------------------------------------
  //
public:
  static bool DeserializeProjectImplStatic(tspProject& spProject, bool bLoadPlugins,
                                           std::function<qint32()> fnFindNewId)
  {
    spProject->m_rwLock.lockForRead();
    const QString sName = spProject->m_sName;
    const QString sFolderName = spProject->m_sFolderName;
    const QString sContentFolder(spProject->m_sProjectPath);
    bool bBundled = spProject->m_bBundled;
    bool bWasLoaded = spProject->m_bLoaded;
    spProject->m_rwLock.unlock();
    bool bOk = true;
    if (!bBundled)
    {
      if (!QFileInfo(sContentFolder + QDir::separator() + sFolderName).exists())
      {
        qWarning() << "Deserialize: Could not find project at: " +
                      sContentFolder + QDir::separator() + sName;
      }
    }

    bOk &= LoadProject(spProject, bLoadPlugins);

    if (bOk)
    {
      QString sJsonFile;
      if (!bBundled)
      {
        sJsonFile = CPhysFsFileEngineHandler::c_sScheme + sFolderName + QDir::separator() +  joip_resource::c_sProjectFileName;
      }
      else
      {
        sJsonFile = ":/" + sName + QDir::separator() + joip_resource::c_sProjectFileName;
      }

      QFileInfo jsonInfo(sJsonFile);
      if (jsonInfo.exists())
      {
        QFile jsonFile(sJsonFile);
        if (jsonFile.open(QIODevice::ReadOnly))
        {
          QJsonParseError err;
          QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonFile.readAll(), &err);
          if (err.error == QJsonParseError::NoError)
          {
            spProject->FromJsonObject(jsonDocument.object());

            spProject->m_rwLock.lockForRead();
            qint32 iOldId = spProject->m_iId;
            QString sProjName = spProject->m_sName;
            SVersion targetVer = spProject->m_iTargetVersion;
            const QString sUserData = spProject->m_sUserData;
            spProject->m_rwLock.unlock();
            // we need to fix naming collisions again here
            QString sError;
            if (!ProjectNameCheck(sProjName, &sError, spProject))
            {
              sProjName = ToValidProjectName(sProjName);
            }

            // we need to check for old EOS projects
            bool bIsOldEos = false;
            QCryptographicHash hash(QCryptographicHash::Algorithm::Sha256);
            const QString sKey = eos::c_sKey + eos::GetEOSK();
            hash.addData(QString(sProjName + sKey).toUtf8());
            if (sUserData == QString::fromUtf8(hash.result().toBase64()))
            {
              bIsOldEos = SVersion(1,7,0) > targetVer;
            }

            qint32 iNewId = (-1 == iOldId) ? fnFindNewId() : -1;
            spProject->m_rwLock.lockForWrite();
            spProject->m_iId = (-1 == iOldId) ? iNewId : iOldId;
            spProject->m_sName = sProjName;
            if (bIsOldEos)
            {
              for (tspScene& spScene : spProject->m_baseData.m_vspScenes)
              {
                QWriteLocker l(&spScene->m_rwLock);
                spScene->m_sceneMode = ESceneMode::eEventDriven;
              }
            }
            spProject->m_rwLock.unlock();
            bOk = true;
          }
          else
          {
            qWarning() << err.errorString();
            bOk = false;
          }
        }
        else
        {
          qWarning() << "Could not read project file: " + sJsonFile;
          bOk = false;
        }
      }
      else
      {
        qWarning() << "Could not find project file: " + sJsonFile;
        bOk = false;
      }
    }

    if (!bWasLoaded)
    {
      bOk &= UnloadProject(spProject);
    }

    return bOk;
  }

  //--------------------------------------------------------------------------------------
  //
protected:
  void LoadProjects(const QString& sUrl, tfnAddProject fnOnAdd, tfnAfterLoad fnAfterLoad) override
  {
    LoadProjectsStatic(sUrl, fnOnAdd, fnAfterLoad);
  }

  //--------------------------------------------------------------------------------------
  //
public:
  static void LoadProjectsStatic(const QString& sUrl, tfnAddProject fnOnAdd, tfnAfterLoad fnAfterLoad)
  {
    // load projects
    // first load folders
    QDirIterator it(sUrl, QDir::Dirs | QDir::NoDotAndDotDot);
    while (it.hasNext())
    {
      QString sDirName = QFileInfo(it.next()).absoluteFilePath();
      fnOnAdd(sDirName, 1, false, false);
    }

    // next load archives
    QStringList vsFileEndings;
    for (const QString& sEnding : CPhysFsFileEngineHandler::SupportedFileTypes())
    {
      vsFileEndings << QStringLiteral("*.") + sEnding.toLower();
    }
    QDirIterator itCompressed(sUrl, vsFileEndings,
                              QDir::Files | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
    while (itCompressed.hasNext())
    {
      QString sFileName = QFileInfo(itCompressed.next()).absoluteFilePath();
      fnOnAdd(sFileName, 1, false, true);
    }

    // finally load packed projects
    vsFileEndings = QStringList() << QString("*") + c_sProjectBundleFileEnding;
    QDirIterator itBundle(sUrl, vsFileEndings,
                          QDir::Files | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
    while (itBundle.hasNext())
    {
      QString sFileName = QFileInfo(itBundle.next()).absoluteFilePath();
      fnOnAdd(sFileName, 1, true, true);
    }

    fnAfterLoad();
  }

  //--------------------------------------------------------------------------------------
  //
protected:
  void LoadKinks() override
  {
    QMutexLocker locker(m_spData.get());

    // load kinks
    qint32 iKinkIdCounter = 0;
    QFile kinkData(":/resources/data/Kink_Information_Data.csv");
    if (kinkData.exists() && kinkData.open(QIODevice::ReadOnly))
    {
      QTextStream stream(&kinkData);
      stream.setCodec("UTF-8");

      QString sLine;
      while (stream.readLineInto(&sLine))
      {
        QStringList vsLineData = sLine.split(";");
        if (vsLineData.size() == 3)
        {
          tKinks& kinks = m_spData->m_kinkKategoryMap[vsLineData[0]];
          tspKink spKink = std::make_shared<SKink>();
          spKink->m_iIdForOrdering = iKinkIdCounter;
          spKink->m_sType = vsLineData[0];
          spKink->m_sName = vsLineData[1];
          spKink->m_sDescribtion = vsLineData[2];
          kinks.insert({vsLineData[1], spKink});
        }
        ++iKinkIdCounter;
      }
    }
  }

  //--------------------------------------------------------------------------------------
  //
  bool PrepareProjectImpl(tspProject& spProject) override
  {
    QReadLocker locker(&spProject->m_rwLock);
    const QString sProjectFolder = PhysicalProjectName(spProject);
    const QString sProjectPath = PhysicalProjectPath(spProject);
    const QString sFolderPath = spProject->m_sProjectPath;

    if (!QFileInfo(sProjectPath).exists())
    {
      bool bOk = QDir(sFolderPath).mkdir(sProjectFolder);
      if (!bOk)
      {
        qWarning() << "Could not create folder: " + sProjectPath;
      }
      return bOk;
    }
    return true;
  }

  //--------------------------------------------------------------------------------------
  //
  bool SerializeProjectImpl(tspProject& spProject, bool bForceWriting) override
  {
    spProject->m_rwLock.lockForRead();
    const QString sName = spProject->m_sName;
    const QString sProjectPath = spProject->m_sProjectPath;
    const QString sFolderName = spProject->m_sFolderName;
    const QString sBaseName = QFileInfo(sFolderName).completeBaseName();
    const QString sSuffix = QFileInfo(sFolderName).suffix();
    bool bBundled = spProject->m_bBundled;
    spProject->m_rwLock.unlock();

    // cannot serialize bundled projects, since these are read only
    if (bBundled) { return true; }

    bool bOk = true;
    QDir sContentFolder(sProjectPath);

    // first rename old folder
    QString sNewFolderName = sFolderName;
    const QString sOldProjectFolder = sProjectPath + QDir::separator() + sFolderName;
    QString sNewProjectFolder = sProjectPath + QDir::separator() + sNewFolderName;
    if (sBaseName != sName)
    {
      if (!sBaseName.isEmpty())
      {
        if (QFileInfo(sOldProjectFolder).exists())
        {
          sNewFolderName = sName + (sSuffix.isEmpty() ? "" : "." + sSuffix);
          sNewProjectFolder = sProjectPath + QDir::separator() + sNewFolderName;
          bOk = sContentFolder.rename(sFolderName, sNewFolderName);
          if (!bOk)
          {
            // reset error so serialization oif the rest doesn't fail
            bOk = true;
            qWarning() << "Serialize: Could not rename folder: " + sOldProjectFolder;
          }
        }
        else
        {
          bOk = false;
          qWarning() << "Serialize: Could not rename folder: " + sOldProjectFolder;
        }
      }
    }

    // if new doesn't exist -> create
    if (bOk && !QFileInfo(sNewProjectFolder).exists())
    {
      bOk = sContentFolder.mkdir(sNewProjectFolder);
      if (!bOk)
      {
        qWarning() << "Serialize: Could not create folder: " +
                      sProjectPath + QDir::separator() + sFolderName;
      }
    }

    bool bWasLoaded = false;
    if (bOk)
    {
      spProject->m_rwLock.lockForWrite();
      spProject->m_sFolderName = sNewFolderName;
      bWasLoaded = spProject->m_bLoaded;
      spProject->m_rwLock.unlock();
    }

    bOk &= LoadProject(spProject, false);

    if (bOk)
    {
      QJsonDocument document(spProject->ToJsonObject());

      QFile jsonFile((!bForceWriting ? CPhysFsFileEngineHandler::c_sScheme :
                                       (sNewProjectFolder + QDir::separator())) +
                     joip_resource::c_sProjectFileName);
      if (jsonFile.open(QIODevice::ReadWrite | QIODevice::Truncate))
      {
        jsonFile.write(document.toJson(QJsonDocument::Indented));
        bOk = true;
      }
      else
      {
        qWarning() << "Could not wirte project file: " + jsonFile.fileName();
        bOk =  false;
      }
    }

    if (!bWasLoaded)
    {
      bOk &= UnloadProject(spProject);
    }

    return bOk;
  }

  //--------------------------------------------------------------------------------------
  //
private:
  std::shared_ptr<CSettings> m_spSettings;
};
#endif

//----------------------------------------------------------------------------------------
//
std::unique_ptr<CDatabaseIO> CDatabaseIO::CreateDatabaseIO(CDatabaseManager* pManager,
                                                           std::shared_ptr<CDatabaseData> spData)
{
#if defined(Q_OS_WIN) || defined(Q_OS_UNIX) || defined(Q_OS_ANDROID)
  return std::make_unique<CDatabaseIODefault>(pManager, spData);
#else
  static_assert(false);
  return nullptr;
#endif
}

//----------------------------------------------------------------------------------------
//
bool CDatabaseIO::LoadBundle(tspProject& spProject, const QString& sBundle)
{
  if (nullptr == spProject || sBundle.isEmpty()) { return false; }

  bool bLoaded = true;

  const QString sProjectName = PhysicalProjectName(spProject);
  QReadLocker locker(&spProject->m_rwLock);

  const auto& it = spProject->m_baseData.m_spResourceBundleMap.find(sBundle);
  if (spProject->m_baseData.m_spResourceBundleMap.end() != it)
  {
    QWriteLocker lockerBundle(&it->second->m_rwLock);
    if (it->second->m_bLoaded) { return true; }

    lockerBundle.unlock();
    const QString sPath = ResourceBundleUrlToAbsolutePath(it->second);
    lockerBundle.relock();

    it->second->m_bLoaded =
        QResource::registerResource(sPath, QDir::separator() + spProject->m_sName);
    //assert(it->second->m_bLoaded); // remove for download testing

    bLoaded = it->second->m_bLoaded;
  }

  return bLoaded;
}

//----------------------------------------------------------------------------------------
//
bool CDatabaseIO::LoadPlugins(tspProject& spProject)
{
  const QString sProjectPath = PhysicalProjectPath(spProject);
  QWriteLocker locker(&spProject->m_rwLock);
  bool bOkLoad = true;

  spProject->m_vspPlugins.clear();
  spProject->m_pluginData.Clear();

  if (!spProject->m_sPluginFolder.isEmpty())
  {
    QFileInfo parentInfo(sProjectPath);
    QFileInfo childInfo(sProjectPath + "/" +  spProject->m_sPluginFolder);

    QDir parentDir(parentInfo.canonicalFilePath());
    QDir childDir(childInfo.canonicalFilePath());

    // Check if the folder is actually inside the project, we don't want plugin folders
    // to be outside.
    if (childDir.absolutePath().startsWith(parentDir.absolutePath() + "/"))
    {
      qint32 iIds = 0;
      QString sPluginFolder = spProject->m_sPluginFolder;
      locker.unlock();
      CDatabaseIODefault::LoadProjectsStatic(sProjectPath + "/" + sPluginFolder,
          [&](const QString& sDirName, quint32 iVersion,
              bool bBundled, bool bReadOnly){
            // TODO: refactor: This does the same as CDatabaseManager::AddProject and
            // CDatabaseManager::AddProjectPrivate combined
            QFileInfo info(sDirName);

            qint32 iNewId = iIds++;
            const QString sBaseName = info.completeBaseName();
            const QString sProjectPath = info.absolutePath();
            QString sName = sBaseName;
            QString sDirNameResolved = sBaseName + "." + info.suffix();
            QString sError;
            if (!ProjectNameCheck(sBaseName, &sError))
            {
              sName = ToValidProjectName(sBaseName);
            }
            else
            {
              sName = sBaseName;
            }

            locker.relock();
            spProject->m_vspPlugins.push_back(std::make_shared<SProject>());
            tspProject spProjNew = spProject->m_vspPlugins.back();
            locker.unlock();
            spProjNew->m_iId = iNewId;
            spProjNew->m_sName = sName;
            spProjNew->m_sFolderName = sDirNameResolved;
            spProjNew->m_sProjectPath = sProjectPath;
            spProjNew->m_iVersion = iVersion;
            spProjNew->m_bBundled = bBundled;
            spProjNew->m_bReadOnly = bReadOnly;
            spProjNew->m_sPlayerLayout = "qrc:/qml/resources/qml/JoipEngine/PlayerDefaultLayout.qml";
            return iNewId;
          },
          [&](){
            locker.relock();
            for (tspProject spProject : spProject->m_vspPlugins)
            {
              locker.unlock();
              CDatabaseIODefault::DeserializeProjectImplStatic(spProject, true, [&](){
                return iIds;
              });
              locker.relock();
            }
            locker.unlock();
          });

      locker.relock();
      for (tspProject& spProjPlug : spProject->m_vspPlugins)
      {
        locker.unlock();
        bool bOkLoadLoc = LoadProject(spProjPlug, true);
        bOkLoad &= bOkLoadLoc;
        locker.relock();

        QReadLocker l(&spProjPlug->m_rwLock);
        if (!bOkLoadLoc)
        {
          qWarning() << "Could not load plugin(s)" << spProjPlug->m_sName <<
              "for Project" << spProject->m_sName;
        }
        else
        {
          spProject->m_pluginData.MergeIntoThis(spProjPlug->m_baseData, spProject);
          spProject->m_pluginData.MergeIntoThis(spProjPlug->m_pluginData, spProject);
        }
      }
    }
    else
    {
      bOkLoad = false;
    }
  }

  return bOkLoad;
}

//----------------------------------------------------------------------------------------
//
bool CDatabaseIO::LoadProject(tspProject& spProject, bool bLoadPlugins)
{
  if (nullptr == spProject) { return false; }

  const QString sProjectPath = PhysicalProjectPath(spProject);
  QWriteLocker locker(&spProject->m_rwLock);
  bool bLoaded = false;
  bool bWasLoaded = spProject->m_bLoaded;
  if (spProject->m_bBundled && !spProject->m_bLoaded)
  {
    spProject->m_bLoaded =
        QResource::registerResource(sProjectPath,
                                    QDir::separator() + spProject->m_sName);
    assert(spProject->m_bLoaded);
    bLoaded = spProject->m_bLoaded;
  }
  else
  {
    if (!spProject->m_bLoaded)
    {
      locker.unlock();
      bool bMountOk = MountProject(spProject);
      locker.relock();
      spProject->m_bLoaded = bMountOk;
      bLoaded = spProject->m_bLoaded;
    }
    else
    {
      bLoaded = true;
    }
  }

  // if loaded, pre-load nessessary resources
  if (!bWasLoaded)
  {
    for (auto& itRes : spProject->m_baseData.m_spResourcesMap)
    {
      tspResource& spRes = itRes.second;
      locker.unlock();
      LoadResource(spRes);
      locker.relock();
    }
  }

  // If load plugins is requested we always load them even if the project was already loaded
  locker.unlock();
  if (bLoadPlugins)
  {
    LoadPlugins(spProject);
  }

  return bLoaded;
}

//----------------------------------------------------------------------------------------
//
bool CDatabaseIO::SetProjectEditing(tspProject& spProject, bool bEnabled)
{
  if (nullptr != spProject)
  {
    const QString sProjPath = PhysicalProjectPath(spProject);
    if (bEnabled)
    {
      return CPhysFsFileEngine::setWriteDir(QString(sProjPath).toStdString().data());
    }
    else
    {
      return CPhysFsFileEngine::setWriteDir(nullptr);
    }
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
bool CDatabaseIO::UnloadBundle(tspProject& spProject, const QString& sBundle)
{
  if (nullptr == spProject || sBundle.isEmpty()) { return false; }

  bool bUnloaded = false;

  const QString sProjectName = PhysicalProjectName(spProject);
  QReadLocker locker(&spProject->m_rwLock);

  const auto& it = spProject->m_baseData.m_spResourceBundleMap.find(sBundle);
  if (spProject->m_baseData.m_spResourceBundleMap.end() != it)
  {
    QWriteLocker lockerBundle(&it->second->m_rwLock);
    if (!it->second->m_bLoaded) { return true; }

    lockerBundle.unlock();
    const QString sPath = ResourceBundleUrlToAbsolutePath(it->second);
    lockerBundle.relock();

    it->second->m_bLoaded =
        !QResource::unregisterResource(sPath, QDir::separator() + spProject->m_sName);
    assert(!it->second->m_bLoaded);

    bUnloaded = !it->second->m_bLoaded;
  }

  return bUnloaded;
}

//----------------------------------------------------------------------------------------
//
bool CDatabaseIO::UnloadPlugins(tspProject& spProject)
{
  QWriteLocker l(&spProject->m_rwLock);
  for (tspProject& spProj : spProject->m_vspPlugins)
  {
    UnloadProject(spProj);
  }
  spProject->m_vspPlugins.clear();
  spProject->m_pluginData.Clear();
  return true;
}

//----------------------------------------------------------------------------------------
//
bool CDatabaseIO::UnloadProject(tspProject& spProject)
{
  if (nullptr == spProject) { return false; }

  const QString sProjectPath = PhysicalProjectPath(spProject);
  QWriteLocker locker(&spProject->m_rwLock);


  // if loaded, unload nessessary resources and plugins
  if (spProject->m_bLoaded)
  {
    UnloadPlugins(spProject);

    for (auto& itRes : spProject->m_baseData.m_spResourcesMap)
    {
      tspResource& spRes = itRes.second;
      UnloadResource(spRes);
    }
  }

  bool bUnloaded = true;

  // if loaded unload bundles
  for (const auto& it : spProject->m_baseData.m_spResourceBundleMap)
  {
    locker.unlock();
    bUnloaded &= UnloadBundle(spProject, it.first);
    locker.relock();
  }


  if (spProject->m_bBundled && spProject->m_bLoaded)
  {
    spProject->m_bLoaded =
        !QResource::unregisterResource(sProjectPath,
                                       QDir::separator() + spProject->m_sName);
    assert(!spProject->m_bLoaded);
    bUnloaded &= !spProject->m_bLoaded;
  }
  else
  {
    locker.unlock();
    if (spProject->m_bLoaded)
    {
      spProject->m_bLoaded = !UnmountProject(spProject);
      bUnloaded &= !spProject->m_bLoaded;
    }
    else
    {
      bUnloaded &= true;
    }
  }

  return bUnloaded;
}

//----------------------------------------------------------------------------------------
//
void CDatabaseIO::LoadResource(tspResource& spRes)
{
  if (nullptr != spRes)
  {
    QWriteLocker lockerRes(&spRes->m_rwLock);
    tspProject spProject = spRes->m_spParent;
    const QString sBundle = spRes->m_sResourceBundle;
    switch(spRes->m_type)
    {
      case EResourceType::eFont:
      {
        lockerRes.unlock();
        // load bundle if needed
        CDatabaseManager::LoadBundle(spProject, sBundle);
        // load font
        qint32 iId = QFontDatabase::addApplicationFont(spRes->ResourceToAbsolutePath());
        lockerRes.relock();
        if (-1 == iId)
        {
          qWarning() << "Could not load font: " << spRes->m_sName;
        }
        spRes->m_iLoadedId = iId;
      } break;
      default: break;
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CDatabaseIO::UnloadResource(tspResource& spRes)
{
  if (nullptr != spRes)
  {
    QWriteLocker lockerRes(&spRes->m_rwLock);
    switch(spRes->m_type)
    {
      case EResourceType::eFont:
      {
        if (-1 != spRes->m_iLoadedId)
        {
          bool bOk = QFontDatabase::removeApplicationFont(spRes->m_iLoadedId);
          if (!bOk)
          {
            qWarning() << "Could not unload font: " << spRes->m_sName;
          }
          spRes->m_iLoadedId = -1;
        }
      } break;
      default: break;
    }
  }
}

//----------------------------------------------------------------------------------------
//
bool CDatabaseIO::MountProject(tspProject& spProject)
{
  QString sFolderName;
  {
    QReadLocker locker(&spProject->m_rwLock);
    sFolderName = spProject->m_sFolderName;
  }
  QString sFinalPath = PhysicalProjectPath(spProject);
  bool bOk = CPhysFsFileEngine::mount(sFinalPath.toStdString().data(),
                                      sFolderName.toStdString().data());
  assert(bOk);
  if (!bOk)
  {
    qWarning() << QObject::tr("Failed to mount %1: reason: %2").arg(sFinalPath)
                  .arg(CPhysFsFileEngine::errorString());
  }
  else
  {
    QReadLocker locker(&spProject->m_rwLock);
    for (const QString& sMountPoint : qAsConst(spProject->m_vsMountPoints))
    {
      bOk &= CPhysFsFileEngine::mount((sFinalPath + "/" + sMountPoint).toStdString().data(), sMountPoint.toStdString().data());
      assert(bOk);
      if (!bOk)
      {
        qWarning() << QObject::tr("Failed to mount %1: reason: %2").arg(sFinalPath)
                      .arg(CPhysFsFileEngine::errorString());
        break;
      }
    }
  }
  return bOk;
}

//----------------------------------------------------------------------------------------
//
bool CDatabaseIO::UnmountProject(tspProject& spProject)
{
  QString sFinalPath = PhysicalProjectPath(spProject);
  bool bOk = true;
  {
    QReadLocker locker(&spProject->m_rwLock);
    for (const QString& sMountPoint : qAsConst(spProject->m_vsMountPoints))
    {
      bOk &= CPhysFsFileEngine::unmount((sFinalPath + "/" + sMountPoint).toStdString().data());
      assert(bOk);
      if (!bOk)
      {
        qWarning() << QObject::tr("Failed to unmount %1: reason: %2").arg(sFinalPath)
                      .arg(CPhysFsFileEngine::errorString());
        break;
      }
    }
  }
  if (!bOk) { return bOk; }

  bOk &= CPhysFsFileEngine::unmount(sFinalPath.toStdString().data());
  assert(bOk);
  if (!bOk)
  {
    qWarning() << QObject::tr("Failed to unmount %1: reason: %2").arg(sFinalPath)
                  .arg(CPhysFsFileEngine::errorString());
  }

  return bOk;
}

//----------------------------------------------------------------------------------------
//
CDatabaseIO::CDatabaseIO(CDatabaseManager* pManager, std::shared_ptr<CDatabaseData> spData) :
  m_pManager(pManager),
  m_spData(spData),
  m_bLoadedDb(0)
{
}

//----------------------------------------------------------------------------------------
//
bool CDatabaseIO::AddResourceArchive(tspProject& spProj, const QUrl& sPath)
{
  const QString sProjPath = PhysicalProjectPath(spProj);
  const QString sMountPoint = sPath.path();
  const QString sFileSuffix = QFileInfo(sPath.fileName()).suffix();
  QWriteLocker locker(&spProj->m_rwLock);
  // resource bundle
  if (joip_resource::c_sResourceBundleSuffix == sFileSuffix)
  {
    const QString sName = QFileInfo(sPath.fileName()).completeBaseName();
    tspResourceBundle spResourceBundle = std::make_shared<SResourceBundle>();
    spResourceBundle->m_spParent = spProj;
    spResourceBundle->m_sName = sName;
    spResourceBundle->m_sPath = sPath;
    spProj->m_baseData.m_spResourceBundleMap.insert({sName, spResourceBundle});
  }
  // archive
  else
  {
    spProj->m_vsMountPoints << sMountPoint;
    if (spProj->m_bLoaded)
    {
      bool bOk = CPhysFsFileEngine::mount((sProjPath + "/" + sMountPoint).toStdString().data(), sMountPoint.toStdString().data());
      if (!bOk)
      {
        qWarning() << QObject::tr("Failed to mount %1: reason: %2").arg(sMountPoint)
                      .arg(CPhysFsFileEngine::errorString());
      }
      return bOk;
    }
  }
  return true;
}

//----------------------------------------------------------------------------------------
//
bool CDatabaseIO::DeserializeProject(tspProject& spProject)
{
  return DeserializeProjectImpl(spProject);
}

//----------------------------------------------------------------------------------------
//
void CDatabaseIO::LoadDatabase()
{
  LoadProjects(ContentPath(),
      [this](const QString& sDirName, quint32 iVersion,
                                     bool bBundled, bool bReadOnly){
        return m_pManager->AddProject(sDirName, iVersion, bBundled, bReadOnly);
      },
      [this](){
        // store projects
        QMutexLocker locker(m_spData.get());
        for (tspProject spProject : m_spData->m_vspProjectDatabase)
        {
          DeserializeProject(spProject);
        }
    });
  LoadKinks();
  m_bLoadedDb = 1;
}

//----------------------------------------------------------------------------------------
//
bool CDatabaseIO::PrepareProject(tspProject& spProject)
{
  return PrepareProjectImpl(spProject);
}

//----------------------------------------------------------------------------------------
//
bool CDatabaseIO::SerializeProject(tspProject& spProject, bool bForceWriting)
{
  return SerializeProjectImpl(spProject, bForceWriting);
}

//----------------------------------------------------------------------------------------
//
void CDatabaseIO::SetDbLoaded(bool bLoaded)
{
  m_bLoadedDb = bLoaded ? 1 : 0;
}
