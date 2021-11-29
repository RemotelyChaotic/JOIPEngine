#include "Application.h"
#include "ClipboardQmlWrapper.h"
#include "Enums.h"
#include "Style.h"
#include "UISoundEmitter.h"

// needed to register to qml
#include "Player/MetronomePaintedWidget.h"
#include "Player/SceneMainScreen.h"
#include "Player/TeaseStorage.h"
#include "Player/TimerWidget.h"

#include "Systems/DatabaseManager.h"
#include "Systems/HelpFactory.h"
#include "Systems/NotificationSender.h"
#include "Systems/OverlayManager.h"
#include "Systems/Project.h"
#include "Systems/ProjectDownloader.h"
#include "Systems/Resource.h"
#include "Systems/Scene.h"
#include "Systems/ScriptRunner.h"
#include "Systems/ThreadedSystem.h"

// needed to register to qml
#include "Systems/Script/ScriptBackground.h"
#include "Systems/Script/ScriptEval.h"
#include "Systems/Script/ScriptIcon.h"
#include "Systems/Script/ScriptMediaPlayer.h"
#include "Systems/Script/ScriptMetronome.h"
#include "Systems/Script/ScriptNotification.h"
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
  m_spSystemsMap.insert({ECoreSystems::eDatabaseManager, std::make_shared<CThreadedSystem>()});
  m_spSystemsMap.insert({ECoreSystems::eProjectDownloader, std::make_shared<CThreadedSystem>()});

  // init subsystems
  m_spSystemsMap[ECoreSystems::eDatabaseManager]->RegisterObject<CDatabaseManager>();
  m_spSystemsMap[ECoreSystems::eProjectDownloader]->RegisterObject<CProjectDownloader>(vJobCfg);

  // qml
  RegisterQmlTypes();

  // style
  LoadStyle();

  StyleHotloadChanged();

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CApplication::SendNotification(const QString& sMsg)
{
  m_spNotifier->SendNotification(sMsg);
}

//----------------------------------------------------------------------------------------
//
template<>
std::weak_ptr<CDatabaseManager> CApplication::System<CDatabaseManager>()
{
  return std::static_pointer_cast<CDatabaseManager>(m_spSystemsMap[ECoreSystems::eDatabaseManager]->Get());
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
    joip_style::SetStyle(this);
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

  qmlRegisterType<CTeaseStorage>("JOIP.core", 1, 1, "TeaseStorage");

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

  qmlRegisterUncreatableType<CKink>("JOIP.db", 1, 1, "Kink", "");
  qmlRegisterUncreatableType<CProjectScriptWrapper>("JOIP.db", 1, 1, "Project", "");
  qmlRegisterUncreatableType<CSceneScriptWrapper>("JOIP.db", 1, 1, "Scene", "");
  qmlRegisterUncreatableType<CResourceScriptWrapper>("JOIP.db", 1, 1, "Resource", "");

  qmlRegisterType<CBackgroundSignalEmitter>("JOIP.script", 1, 1, "BackgroundSignalEmitter");
  qmlRegisterType<CEvalSignalEmiter>("JOIP.script", 1, 1, "EvalSignalEmitter");
  qmlRegisterType<CIconSignalEmitter>("JOIP.script", 1, 1, "IconSignalEmitter");
  qmlRegisterType<CMediaPlayerSignalEmitter>("JOIP.script", 1, 1, "MediaPlayerSignalEmitter");
  qmlRegisterType<CMetronomeSignalEmitter>("JOIP.script", 1, 1, "MetronomeSignalEmitter");
  qmlRegisterType<CNotificationSignalEmiter>("JOIP.script", 1, 1, "NotificationSignalEmiter");
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

