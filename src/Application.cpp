#include "Application.h"
#include "Enums.h"
#include "Style.h"
#include "UISoundEmitter.h"

// needed to register to qml
#include "Player/TimerWidget.h"

#include "Systems/DatabaseManager.h"
#include "Systems/HelpFactory.h"
#include "Systems/OverlayManager.h"
#include "Systems/Project.h"
#include "Systems/Resource.h"
#include "Systems/Scene.h"
#include "Systems/ScriptRunner.h"
#include "Systems/ThreadedSystem.h"

// needed to register to qml
#include "Systems/Script/ScriptBackground.h"
#include "Systems/Script/ScriptIcon.h"
#include "Systems/Script/ScriptMediaPlayer.h"
#include "Systems/Script/ScriptStorage.h"
#include "Systems/Script/ScriptTextBox.h"
#include "Systems/Script/ScriptThread.h"
#include "Systems/Script/ScriptTimer.h"
// needed to register to qml
#include <filters/regexpfilter.h>
#include <sorters/stringsorter.h>
#include <qqmlsortfilterproxymodel.h>

#include <QDebug>
#include <QFontDatabase>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQmlWebChannel>
#include <cassert>

CApplication::CApplication(int& argc, char *argv[]) :
  QApplication(argc, argv),
  m_spSystemsMap(),
  m_spSoundEmitter(std::make_unique<CUISoundEmitter>()),
  m_spHelpFactory(std::make_unique<CHelpFactory>()),
  m_spOverlayManager(std::make_shared<COverlayManager>()),
  m_spSettings(nullptr),
  m_bStyleDirty(true),
  m_bInitialized(false)
{

}

CApplication::~CApplication()
{
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
  qint32 iFont = QFontDatabase::addApplicationFont("://resources/fonts/Equestria.otf");
  Q_ASSERT(-1 != iFont);
  Q_UNUSED(iFont)

  QStringList fonts;
  QFontDatabase fontDb;
  for(int i=0; i< fontDb.families().size(); i++)
  {
    fonts << fontDb.families().at(i);
  }
  //qDebug() << QString(tr("Supported font families: %1")).arg(fonts.join(", "));

  // settings
  m_spSettings = std::make_shared<CSettings>();
  connect(m_spSettings.get(), &CSettings::fontChanged,
          this, &CApplication::MarkStyleDirty, Qt::DirectConnection);
  connect(m_spSettings.get(), &CSettings::styleChanged,
          this, &CApplication::MarkStyleDirty, Qt::DirectConnection);

  // sound emitter
  m_spSoundEmitter->Initialize();

  // help factory
  m_spHelpFactory->Initialize();

  // Overlay Manager
  m_spOverlayManager->Initialize();

  // create subsystems
  m_spSystemsMap.insert({ECoreSystems::eDatabaseManager, std::make_shared<CThreadedSystem>()});
  m_spSystemsMap.insert({ECoreSystems::eScriptRunner, std::make_shared<CThreadedSystem>()});

  // init subsystems
  m_spSystemsMap[ECoreSystems::eDatabaseManager]->RegisterObject<CDatabaseManager>();
  m_spSystemsMap[ECoreSystems::eScriptRunner]->RegisterObject<CScriptRunner>();

  // qml
  RegisterQmlTypes();

  // style
  LoadStyle();

  m_bInitialized = true;
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
std::weak_ptr<CScriptRunner> CApplication::System<CScriptRunner>()
{
  return std::static_pointer_cast<CScriptRunner>(m_spSystemsMap[ECoreSystems::eScriptRunner]->Get());
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
void CApplication::RegisterQmlTypes()
{
  qRegisterMetaType<QColor>();
  qRegisterMetaType<std::vector<QColor>>();
  qRegisterMetaType<QStringList>();
  qRegisterMetaType<QtMsgType>();

  qRegisterMetaType<CResource*>();
  qRegisterMetaType<tspResource>();
  qRegisterMetaType<CScene*>();
  qRegisterMetaType<tspScene>();
  qRegisterMetaType<CProject*>();
  qRegisterMetaType<tspProject>();

  qRegisterMetaType<QQmlWebChannel*>();

  qmlRegisterSingletonType<CSettings>("JOIP.core", 1, 1, "Settings",
                                      [this](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject*
  {
      Q_UNUSED(scriptEngine)
      std::shared_ptr<CSettings> spSettings = Settings();
      engine->setObjectOwnership(spSettings.get(), QQmlEngine::CppOwnership);
      return spSettings.get();
  });

  qmlRegisterSingletonType<CScriptRunner>("JOIP.core", 1, 1, "ScriptRunner",
                                      [this](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject*
  {
      Q_UNUSED(scriptEngine)
      std::shared_ptr<CScriptRunner> spScriptRunner = System<CScriptRunner>().lock();
      engine->setObjectOwnership(spScriptRunner.get(), QQmlEngine::CppOwnership);
      return spScriptRunner.get();
  });

  qmlRegisterUncreatableType<CProject>("JOIP.db", 1, 1, "Project", "");
  qmlRegisterUncreatableType<CScene>("JOIP.db", 1, 1, "Scene", "");
  qmlRegisterUncreatableType<CResource>("JOIP.db", 1, 1, "Resource", "");

  qmlRegisterType<CBackgroundSignalEmitter>("JOIP.script", 1, 1, "BackgroundSignalEmitter");
  qmlRegisterType<CIconSignalEmitter>("JOIP.script", 1, 1, "IconSignalEmitter");
  qmlRegisterType<CMediaPlayerSignalEmitter>("JOIP.script", 1, 1, "MediaPlayerSignalEmitter");
  qmlRegisterType<CStorageSignalEmitter>("JOIP.script", 1, 1, "StorageSignalEmitter");
  qmlRegisterType<CTextBoxSignalEmitter>("JOIP.script", 1, 1, "TextBoxSignalEmitter");
  qmlRegisterType<CThreadSignalEmitter>("JOIP.script", 1, 1, "ThreadSignalEmitter");
  qmlRegisterType<CTimerSignalEmitter>("JOIP.script", 1, 1, "TimerSignalEmitter");

  qmlRegisterType<CTimerCanvasQml>("JOIP.ui", 1, 1, "TimerDisplay");

  qmlRegisterUncreatableType<qqsfpm::Filter>("SortFilterProxyModel", 0, 2, "Filter", "Filter is abstract and cannot be created.");
  qmlRegisterUncreatableType<qqsfpm::Sorter>("SortFilterProxyModel", 0, 2, "Sorter", "Sorter is abstract and cannot be created.");
  qmlRegisterType<qqsfpm::RegExpFilter>("SortFilterProxyModel", 0, 2, "RegExpFilter");
  qmlRegisterType<qqsfpm::StringSorter>("SortFilterProxyModel", 0, 2, "StringSorter");
  qmlRegisterType<qqsfpm::QQmlSortFilterProxyModel>("SortFilterProxyModel", 0, 2, "SortFilterProxyModel");
}

