#include "Application.h"
#include "ClipboardQmlWrapper.h"
#include "Enums.h"
#include "Style.h"
#include "UISoundEmitter.h"

// needed to register to qml
#include "Player/MetronomePaintedWidget.h"
#include "Player/ProjectDialogueManager.h"
#include "Player/ProjectNotificationManager.h"
#include "Player/ProjectSceneManager.h"
#include "Player/ProjectSoundManager.h"
#include "Player/ProjectSavegameManager.h"
#include "Player/SceneMainScreen.h"
#include "Player/TeaseDeviceController.h"
#include "Player/TeaseStorage.h"
#include "Player/TimerWidget.h"

#include "Systems/BackActionHandler.h"
#include "Systems/Debug/DebugInterface.h"
#include "Systems/DatabaseManager.h"
#include "Systems/DeviceManager.h"
#include "Systems/Devices/DeviceSettings.h"
#include "Systems/HelpFactory.h"
#include "Systems/MetronomeManager.h"
#include "Systems/NotificationSender.h"
#include "Systems/OverlayManager.h"
#include "Systems/Database/Project.h"
#include "Systems/ProjectDownloader.h"
#include "Systems/Database/Resource.h"
#include "Systems/Database/Scene.h"
#include "Systems/ScriptRunner.h"
#include "Systems/ThreadedSystem.h"

// needed to register to qml
#include "Systems/Script/ScriptBackground.h"
#include "Systems/Script/ScriptDeviceController.h"
#include "Systems/Script/ScriptDbWrappers.h"
#include "Systems/Script/ScriptEval.h"
#include "Systems/Script/ScriptEventSender.h"
#include "Systems/Script/ScriptIcon.h"
#include "Systems/Script/ScriptMediaPlayer.h"
#include "Systems/Script/ScriptMetronome.h"
#include "Systems/Script/ScriptNotification.h"
#include "Systems/Script/ScriptSceneManager.h"
#include "Systems/Script/ScriptStorage.h"
#include "Systems/Script/ScriptTextBox.h"
#include "Systems/Script/ScriptThread.h"
#include "Systems/Script/ScriptTimer.h"
// needed to register to qml

#include <filters/regexpfilter.h>
#include <sorters/stringsorter.h>
#include <qqmlsortfilterproxymodel.h>

#include <QDebug>
#include <QFileInfo>
#include <QFontDatabase>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQmlWebChannel>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QTimer>
#include <cassert>

CQmlApplicationQtNamespaceWrapper::CQmlApplicationQtNamespaceWrapper(QObject* eParent) :
  QObject(eParent)
{
}
CQmlApplicationQtNamespaceWrapper::~CQmlApplicationQtNamespaceWrapper()
{
}

//----------------------------------------------------------------------------------------
//
QString CQmlApplicationQtNamespaceWrapper::decodeHTML(const QString& sString)
{
  return QTextDocumentFragment::fromHtml(sString).toPlainText();
}

//----------------------------------------------------------------------------------------
//
bool CQmlApplicationQtNamespaceWrapper::mightBeRichtext(const QString& sString)
{
  return Qt::mightBeRichText(sString);
}

//----------------------------------------------------------------------------------------
//
CApplication::CApplication(int& argc, char *argv[]) :
  QApplication(argc, argv),
  m_spSystemsMap(),
  m_spSoundEmitter(std::make_unique<CUISoundEmitter>()),
  m_spHelpFactory(std::make_unique<CHelpFactory>()),
  m_spOverlayManager(std::make_shared<COverlayManager>()),
  m_spSettings(nullptr),
  m_spNotifier(std::make_shared<CNotificationSender>()),
  m_spBackActionHandler(nullptr),
  m_spDebugInterface(nullptr),
  m_styleWatcher(),
  m_bStyleDirty(true),
  m_bInitialized(false)
{

}

CApplication::~CApplication()
{
  if (auto spPrjDownloader = System<CProjectDownloader>().lock())
  {
    spPrjDownloader->ClearQueue();
    spPrjDownloader->StopRunningJobs();
    spPrjDownloader->WaitForFinished();
  }

  m_spOverlayManager->Deinitialize();
  m_spHelpFactory->Deinitialize();
}

//----------------------------------------------------------------------------------------
//
CApplication* CApplication::Instance()
{
  CApplication* pApp = dynamic_cast<CApplication*>(QCoreApplication::instance());
  assert(nullptr != pApp);
  return pApp;
}

//----------------------------------------------------------------------------------------
//
void CApplication::Initialize()
{
  // sound
  installEventFilter(m_spSoundEmitter.get());
  m_spBackActionHandler = std::make_shared<CBackActionHandler>();

  // fonts
  QFontDatabase::addApplicationFont(":/resources/fonts/Equestria.otf");
  QFontDatabase::addApplicationFont(":/resources/fonts/CloisterBlack.ttf");

  QStringList fonts;
  QFontDatabase fontDb;
  for(int i=0; i< fontDb.families().size(); i++)
  {
    fonts << fontDb.families().at(i);
  }
  //qDebug() << QString(tr("Supported font families: %1")).arg(fonts.join(", "));

  // settings
  m_spSettings = std::make_shared<CSettings>();

  // copy host config
  std::vector<SDownloadJobConfig> vJobCfg;
  for (const auto& it : CDownloadJobFactory::GetHostSettingMap())
  {
    if (!m_spSettings->HasRaw(it.second.m_sSettingsEntry))
    {
      m_spSettings->WriteRaw(it.second.m_sSettingsEntry, it.second.m_vsAllowedHosts);
    }
    QVariant varRet = m_spSettings->ReadRaw(it.second.m_sSettingsEntry, it.second.m_vsAllowedHosts);
    vJobCfg.push_back({it.second.m_sClassType, it.second.m_sSettingsEntry, varRet.toStringList()});
  }

  // create and init debug interface
  m_spDebugInterface = std::make_shared<CDebugInterface>();
  m_spDebugInterface->Register("app", this);

  // create device settings
  CDeviceSettingFactory::InitializeSettings();

  connect(m_spSettings.get(), &CSettings::fontChanged,
          this, &CApplication::MarkStyleDirty, Qt::DirectConnection);
  connect(m_spSettings.get(), &CSettings::styleChanged,
          this, &CApplication::MarkStyleDirty, Qt::DirectConnection);
  connect(m_spSettings.get(), &CSettings::styleHotLoadChanged,
          this, &CApplication::StyleHotloadChanged, Qt::DirectConnection);

  // sound emitter
  m_spSoundEmitter->Initialize();

  // help factory
  m_spHelpFactory->Initialize();

  // Overlay Manager
  m_spOverlayManager->Initialize();

  // create subsystems
  m_spSystemsMap.insert({ECoreSystems::eDatabaseManager, std::make_shared<CThreadedSystem>("DatabaseManager")});
  m_spSystemsMap.insert({ECoreSystems::eProjectDownloader, std::make_shared<CThreadedSystem>("ProjectDownloader")});
  m_spSystemsMap.insert({ECoreSystems::eDeviceManager, std::make_shared<CThreadedSystem>("DeviceManager")});
  m_spSystemsMap.insert({ECoreSystems::eMetronomeManager, std::make_shared<CThreadedSystem>("MetronomeManager")});

  // init subsystems
  m_spSystemsMap[ECoreSystems::eDatabaseManager]->RegisterObject<CDatabaseManager>();
  m_spSystemsMap[ECoreSystems::eProjectDownloader]->RegisterObject<CProjectDownloader>(vJobCfg);
  m_spSystemsMap[ECoreSystems::eDeviceManager]->RegisterObject<CDeviceManager>();
  m_spSystemsMap[ECoreSystems::eMetronomeManager]->RegisterObject<CMetronomeManager>();

  // qml
  RegisterQmlTypes();

  // style
  LoadStyle();

  StyleHotloadChanged();

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CApplication::SendNotification(const QString& sTitle, const QString& sMsg)
{
  m_spNotifier->SendNotification(sTitle, sMsg);
}

//----------------------------------------------------------------------------------------
//
template<>
std::weak_ptr<CDatabaseManager> CApplication::System<CDatabaseManager>()
{
  return std::static_pointer_cast<CDatabaseManager>(m_spSystemsMap[ECoreSystems::eDatabaseManager]->Get());
}

template<>
std::weak_ptr<CDeviceManager> CApplication::System<CDeviceManager>()
{
  return std::static_pointer_cast<CDeviceManager>(m_spSystemsMap[ECoreSystems::eDeviceManager]->Get());
}

template<>
std::weak_ptr<CHelpFactory> CApplication::System<CHelpFactory>()
{
  return m_spHelpFactory;
}

template<>
std::weak_ptr<COverlayManager> CApplication::System<COverlayManager>()
{
  return m_spOverlayManager;
}

template<>
std::weak_ptr<CProjectDownloader> CApplication::System<CProjectDownloader>()
{
  return std::static_pointer_cast<CProjectDownloader>(m_spSystemsMap[ECoreSystems::eProjectDownloader]->Get());
}

template<>
std::weak_ptr<CBackActionHandler> CApplication::System<CBackActionHandler>()
{
  return m_spBackActionHandler;
}
template<>
std::weak_ptr<CMetronomeManager> CApplication::System<CMetronomeManager>()
{
  return std::static_pointer_cast<CMetronomeManager>(m_spSystemsMap[ECoreSystems::eMetronomeManager]->Get());
}

//----------------------------------------------------------------------------------------
//
void CApplication::MarkStyleDirty()
{
  m_bStyleDirty = true;
  QTimer::singleShot(500, this, &CApplication::LoadStyle);
}

//----------------------------------------------------------------------------------------
//
void CApplication::LoadStyle()
{
  if (m_bStyleDirty)
  {
    m_bStyleDirty = false;
    joip_style::SetStyle(this, m_spSettings->Style(), m_spSettings->Font());
    emit StyleLoaded();
  }
}

//----------------------------------------------------------------------------------------
//
void CApplication::StyleHotloadChanged()
{
  if (m_spSettings->StyleHotLoad())
  {
    const QString sString = joip_style::StyleFile(m_spSettings->Style());
    if (QFileInfo(sString).exists())
    {
      m_styleWatcher.addPath(sString);
      connect(&m_styleWatcher, &QFileSystemWatcher::fileChanged,
              this, &CApplication::MarkStyleDirty, Qt::DirectConnection);
    }
  }
  else
  {
    m_styleWatcher.removePaths(m_styleWatcher.files());
    disconnect(&m_styleWatcher, &QFileSystemWatcher::fileChanged,
               this, &CApplication::MarkStyleDirty);
  }
}

//----------------------------------------------------------------------------------------
//
void CApplication::RegisterQmlTypes()
{
  qRegisterMetaType<QColor>();
  qRegisterMetaType<std::vector<QColor>>();
  qRegisterMetaType<QStringList>();
  qRegisterMetaType<QList<int>>();
  qRegisterMetaType<QtMsgType>();
  qRegisterMetaType<Qt::ApplicationState>();

  qRegisterMetaType<CResourceScriptWrapper*>();
  qRegisterMetaType<tspResource>();
  qRegisterMetaType<CSceneScriptWrapper*>();
  qRegisterMetaType<tspScene>();
  qRegisterMetaType<CProjectScriptWrapper*>();
  qRegisterMetaType<tspProject>();

  qRegisterMetaType<QQmlWebChannel*>();

  qmlRegisterType<CClipboardQmlWrapper>("QtGui", 5, 14, "Clipboard");

  qmlRegisterType<CTeaseStorageWrapper>("JOIP.core", 1, 1, "TeaseStorage");
  qmlRegisterUncreatableType<CProjectEventTargetWrapper>("JOIP.core", 1, 2, "EventTarget", "");
  qmlRegisterUncreatableType<CNotificationInstanceWrapper>("JOIP.core", 1, 2, "NotificationInstance", "");
  qmlRegisterType<CProjectSceneManagerWrapper>("JOIP.core", 1, 2, "SceneManager");
  qmlRegisterUncreatableType<CSoundInstanceWrapper>("JOIP.core", 1, 2, "SoundInstance", "");
  qmlRegisterType<CProjectSavegameManager>("JOIP.core", 1, 5, "SavegameManager");

  qmlRegisterUncreatableMetaObject(
      DominantHand::staticMetaObject, "JOIP.core", 1, 1, "DominantHand", "");

  qmlRegisterSingletonType<CQmlApplicationQtNamespaceWrapper>(
        "JOIP.core", 1, 1, "QtApp",
        [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject*
  {
      Q_UNUSED(scriptEngine)
      if (nullptr != engine)
      {
        return new CQmlApplicationQtNamespaceWrapper(engine);
      }
      else if (nullptr != scriptEngine)
      {
        return new CQmlApplicationQtNamespaceWrapper(scriptEngine);
      }
      return nullptr;
  });
  qmlRegisterSingletonType<CSettings>("JOIP.core", 1, 1, "Settings",
                                      [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject*
  {
      Q_UNUSED(scriptEngine)
      if (nullptr != engine)
      {
        return new CSettings(engine);
      }
      else if (nullptr != scriptEngine)
      {
        return new CSettings(scriptEngine);
      }
      return nullptr;
  });
  qmlRegisterSingletonType<CScriptRunnerWrapper>("JOIP.core", 1, 1, "ScriptRunner",
                                                 [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject*
  {
      Q_UNUSED(scriptEngine)
      if (nullptr != engine)
      {
        CSceneMainScreen* pMainScreen = engine->property(player::c_sMainPlayerProperty).value<CSceneMainScreen*>();
        if (nullptr != pMainScreen)
        {
          return new CScriptRunnerWrapper(engine, pMainScreen->ScriptRunner());
        }
      }
      else if (nullptr != scriptEngine)
      {
        CSceneMainScreen* pMainScreen = engine->property(player::c_sMainPlayerProperty).value<CSceneMainScreen*>();
        if (nullptr != pMainScreen)
        {
          return new CScriptRunnerWrapper(scriptEngine, pMainScreen->ScriptRunner());
        }
      }
      return nullptr;
  });
  qmlRegisterSingletonType<CSceneMainScreenWrapper>("JOIP.core", 1, 2, "Player",
                                                    [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject*
  {
      Q_UNUSED(scriptEngine)
      if (nullptr != engine)
      {
        CSceneMainScreen* pMainScreen = engine->property(player::c_sMainPlayerProperty).value<CSceneMainScreen*>();
        if (nullptr != pMainScreen)
        {
          return new CSceneMainScreenWrapper(engine, pMainScreen);
        }
      }
      else if (nullptr != scriptEngine)
      {
        CSceneMainScreen* pMainScreen = engine->property(player::c_sMainPlayerProperty).value<CSceneMainScreen*>();
        if (nullptr != pMainScreen)
        {
          return new CSceneMainScreenWrapper(scriptEngine, pMainScreen);
        }
      }
      return nullptr;
  });
  qmlRegisterSingletonType<CProjectNotificationManager>("JOIP.core", 1, 2, "NotificationManager",
                                                        [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject*
  {
      Q_UNUSED(scriptEngine)
      if (nullptr != engine)
      {
        CSceneMainScreen* pMainScreen = engine->property(player::c_sMainPlayerProperty).value<CSceneMainScreen*>();
        if (nullptr != pMainScreen)
        {
          CProjectNotificationManager* pWrapper = new CProjectNotificationManager(engine, pMainScreen);
          pWrapper->Initalize(pMainScreen->EventCallbackRegistry());
          return pWrapper;
        }
      }
      else if (nullptr != scriptEngine)
      {
        CSceneMainScreen* pMainScreen = engine->property(player::c_sMainPlayerProperty).value<CSceneMainScreen*>();
        if (nullptr != pMainScreen)
        {
          CProjectNotificationManager* pWrapper = new CProjectNotificationManager(engine, pMainScreen);
          pWrapper->Initalize(pMainScreen->EventCallbackRegistry());
          return pWrapper;
        }
      }
      return nullptr;
  });
  qmlRegisterSingletonType<CProjectSoundManager>("JOIP.core", 1, 2, "SoundManager",
                                                 [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject*
  {
      Q_UNUSED(scriptEngine)
      if (nullptr != engine)
      {
        CSceneMainScreen* pMainScreen = engine->property(player::c_sMainPlayerProperty).value<CSceneMainScreen*>();
        if (nullptr != pMainScreen)
        {
          CProjectSoundManager* pWrapper = new CProjectSoundManager(engine, pMainScreen);
          pWrapper->Initalize(pMainScreen->EventCallbackRegistry());
          return pWrapper;
        }
      }
      else if (nullptr != scriptEngine)
      {
        CSceneMainScreen* pMainScreen = engine->property(player::c_sMainPlayerProperty).value<CSceneMainScreen*>();
        if (nullptr != pMainScreen)
        {
          CProjectSoundManager* pWrapper = new CProjectSoundManager(engine, pMainScreen);
          pWrapper->Initalize(pMainScreen->EventCallbackRegistry());
          return pWrapper;
        }
      }
      return nullptr;
  });
  qmlRegisterSingletonType<CTeaseDeviceController>("JOIP.core", 1, 3, "TeaseDeviceController",
                                                 [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject*
  {
    Q_UNUSED(scriptEngine)
    if (nullptr != engine)
    {
      CSceneMainScreen* pMainScreen = engine->property(player::c_sMainPlayerProperty).value<CSceneMainScreen*>();
      if (nullptr != pMainScreen)
      {
        return new CTeaseDeviceController(engine, pMainScreen);
      }
    }
    else if (nullptr != scriptEngine)
    {
      CSceneMainScreen* pMainScreen = engine->property(player::c_sMainPlayerProperty).value<CSceneMainScreen*>();
      if (nullptr != pMainScreen)
      {
        return new CTeaseDeviceController(scriptEngine, pMainScreen);
      }
    }
    return nullptr;
  });
  qmlRegisterSingletonType<CProjectDialogueManagerWrapper>("JOIP.core", 1, 5, "DialogueManager",
                                                   [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject*
  {
    Q_UNUSED(scriptEngine)
    if (nullptr != engine)
    {
      CSceneMainScreen* pMainScreen = engine->property(player::c_sMainPlayerProperty).value<CSceneMainScreen*>();
      if (nullptr != pMainScreen)
      {
        return new CProjectDialogueManagerWrapper(engine, pMainScreen->ProjectDialogueManager());
      }
    }
    else if (nullptr != scriptEngine)
    {
      CSceneMainScreen* pMainScreen = engine->property(player::c_sMainPlayerProperty).value<CSceneMainScreen*>();
      if (nullptr != pMainScreen)
      {
        return new CProjectDialogueManagerWrapper(scriptEngine, pMainScreen->ProjectDialogueManager());
      }
    }
    return nullptr;
  });

  qmlRegisterUncreatableType<CKinkWrapper>("JOIP.db", 1, 1, "Kink", "");
  qmlRegisterUncreatableType<CProjectScriptWrapper>("JOIP.db", 1, 1, "Project", "");
  qmlRegisterUncreatableType<CSceneScriptWrapper>("JOIP.db", 1, 1, "Scene", "");
  qmlRegisterUncreatableType<CResourceScriptWrapper>("JOIP.db", 1, 1, "Resource", "");
  qmlRegisterUncreatableType<CDialogueWrapper>("JOIP.db", 1, 5, "Dialogue", "");
  qmlRegisterUncreatableType<CDialogueDataWrapper>("JOIP.db", 1, 5, "DialogueData", "");
  qmlRegisterUncreatableType<CSaveDataWrapper>("JOIP.db", 1, 5, "SaveData", "");

  qmlRegisterType<CBackgroundSignalEmitter>("JOIP.script", 1, 1, "BackgroundSignalEmitter");
  qmlRegisterType<CDeviceControllerSignalEmitter>("JOIP.script", 1, 3, "DeviceControllerSignalEmitter");
  qmlRegisterType<CEvalSignalEmiter>("JOIP.script", 1, 2, "EvalSignalEmitter");
  qmlRegisterType<CEventSenderSignalEmitter>("JOIP.script", 1, 3, "EventSenderSignalEmitter");
  qmlRegisterType<CIconSignalEmitter>("JOIP.script", 1, 1, "IconSignalEmitter");
  qmlRegisterType<CMediaPlayerSignalEmitter>("JOIP.script", 1, 1, "MediaPlayerSignalEmitter");
  qmlRegisterType<CMetronomeSignalEmitter>("JOIP.script", 1, 1, "MetronomeSignalEmitter");
  qmlRegisterType<CNotificationSignalEmiter>("JOIP.script", 1, 1, "NotificationSignalEmiter");
  qmlRegisterType<CSceneManagerSignalEmiter>("JOIP.script", 1, 2, "SceneManagerSignalEmiter");
  qmlRegisterType<CStorageSignalEmitter>("JOIP.script", 1, 1, "StorageSignalEmitter");
  qmlRegisterType<CTextBoxSignalEmitter>("JOIP.script", 1, 1, "TextBoxSignalEmitter");
  qmlRegisterType<CThreadSignalEmitter>("JOIP.script", 1, 1, "ThreadSignalEmitter");
  qmlRegisterType<CTimerSignalEmitter>("JOIP.script", 1, 1, "TimerSignalEmitter");

  qmlRegisterUncreatableMetaObject(
    IconAlignment::staticMetaObject, "JOIP.script", 1, 1, "IconAlignment", "");
  qmlRegisterUncreatableMetaObject(
    TextAlignment::staticMetaObject, "JOIP.script", 1, 1, "TextAlignment", "");

  qmlRegisterType<CMetronomeCanvasQml>("JOIP.ui", 1, 1, "MetronomeDisplay");
  qmlRegisterType<CTimerCanvasQml>("JOIP.ui", 1, 1, "TimerDisplay");

  qmlRegisterUncreatableType<qqsfpm::Filter>("SortFilterProxyModel", 0, 2, "Filter", "Filter is abstract and cannot be created.");
  qmlRegisterUncreatableType<qqsfpm::Sorter>("SortFilterProxyModel", 0, 2, "Sorter", "Sorter is abstract and cannot be created.");
  qmlRegisterType<qqsfpm::RegExpFilter>("SortFilterProxyModel", 0, 2, "RegExpFilter");
  qmlRegisterType<qqsfpm::StringSorter>("SortFilterProxyModel", 0, 2, "StringSorter");
}

//----------------------------------------------------------------------------------------
//
CSettings* CApplication::SettingsImpl()
{
  return m_spSettings.get();
}
