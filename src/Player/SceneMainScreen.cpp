#include "SceneMainScreen.h"
#include "Application.h"
#include "Constants.h"
#include "ProjectEventTarget.h"
#include "ProjectRunner.h"
#include "ProjectSceneManager.h"
#include "Settings.h"
#include "WindowContext.h"

#include "Systems/BackActionHandler.h"
#include "Systems/DatabaseManager.h"
#include "Systems/DatabaseImageProvider.h"
#include "Systems/Project.h"
#include "Systems/ScriptRunner.h"
#include "Systems/Script/ScriptRunnerSignalEmiter.h"
#include "Systems/ThreadedSystem.h"
#include "Widgets/BackgroundWidget.h"
#include "ui_SceneMainScreen.h"

#include <XmlDomWrapper.h>

#include <QAction>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QResizeEvent>
#include <QUrl>

#include <assert.h>
#include <random>

const char* player::c_sMainPlayerProperty = "MainPlayer";

//----------------------------------------------------------------------------------------
//
CSceneMainScreen::CSceneMainScreen(QWidget* pParent) :
  QWidget(pParent),
  m_spUi(std::make_unique<Ui::CSceneMainScreen>()),
  m_spEventCallbackRegistry(std::make_shared<CProjectEventCallbackRegistry>()),
  m_spProjectRunner(std::make_shared<CProjectRunner>()),
  m_spScriptRunnerSystem(std::make_shared<CThreadedSystem>("ScriptRunner")),
  m_spScriptRunner(nullptr),
  m_spSettings(CApplication::Instance()->Settings()),
  m_spCurrentProject(nullptr),
  m_pCurrentProjectWrapper(nullptr),
  m_wpDbManager(),
  m_lastScriptExecutionStatus(static_cast<qint32>(CScriptRunnerSignalEmiter::ScriptExecStatus::eRunning)),
  m_bInitialized(false),
  m_bShuttingDown(false),
  m_bErrorState(false),
  m_bBeingDebugged(false),
  m_bCloseRequested(false)
{
  m_spUi->setupUi(this);
}

CSceneMainScreen::~CSceneMainScreen()
{
}

//----------------------------------------------------------------------------------------
//
bool CSceneMainScreen::CloseApplication()
{
  if (!m_bCloseRequested)
  {
    bool bOk = QMetaObject::invokeMethod(this, "SlotQuit", Qt::QueuedConnection);
    assert(bOk);
    m_bCloseRequested = bOk;
    return bOk;
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::Initialize(const std::shared_ptr<CWindowContext>& spWindowContext,
                                  bool bDebug)
{
  m_bInitialized = false;

  m_spWindowContext = spWindowContext;

  connect(CApplication::Instance(), &QGuiApplication::applicationStateChanged,
          this, &CSceneMainScreen::SlotApplicationStateChanged);

  connect(m_spEventCallbackRegistry.get(), &CProjectEventCallbackRegistry::SignalError,
          this, &CSceneMainScreen::SlotError);
  connect(m_spProjectRunner.get(), &CProjectRunner::SignalChangeSceneRequest,
          m_spEventCallbackRegistry.get(), [this](const QString&) {
    m_spEventCallbackRegistry->Dispatch("CProjectSceneManagerWrapper", "change");
  });

  m_spScriptRunnerSystem->RegisterObject<CScriptRunner>();
  m_spScriptRunner = std::dynamic_pointer_cast<CScriptRunner>(m_spScriptRunnerSystem->Get());
  assert(m_spScriptRunner != nullptr);
  QQmlEngine::setObjectOwnership(m_spScriptRunner.get(), QQmlEngine::CppOwnership);

  m_wpDbManager = CApplication::Instance()->System<CDatabaseManager>();

  InitQmlMain();

  connect(m_spProjectRunner.get(), &CProjectRunner::SignalChangeSceneRequest,
          this, [this](const QString& sScene) {
    SlotScriptRunFinished(true, sScene);
  });
  connect(m_spProjectRunner.get(), &CProjectRunner::SignalError,
          this, &CSceneMainScreen::SlotError);

  SetDebugging(bDebug);

  // initializing done
  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::LoadProject(qint32 iId, const QString sStartScene)
{
  if (!m_bInitialized) { return; }
  if (nullptr != m_spCurrentProject)
  {
    assert(false);
    qWarning() << "Old Project was not unloaded before loading project.";
  }

  m_spUi->pQmlWidget->setSource(QUrl("qrc:/qml/resources/qml/PlayerMain.qml"));

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    m_spCurrentProject = spDbManager->FindProject(iId);
    if (nullptr == m_spCurrentProject) { return; }

    CDatabaseManager::LoadProject(m_spCurrentProject);
    m_spProjectRunner->LoadProject(m_spCurrentProject, sStartScene);

    if (nullptr != m_spWindowContext && !m_bBeingDebugged)
    {
      m_spWindowContext->SignalChangeAppOverlay(":/resources/style/img/ButtonPlay.png");
    }

    ConnectAllSignals();
    LoadQml();

    m_bShuttingDown = false;
    m_bErrorState = false;
  }

  if (!m_bBeingDebugged)
  {
    if (auto spBackActionHandler = CApplication::Instance()->System<CBackActionHandler>().lock())
    {
      spBackActionHandler->RegisterSlotToCall(this, "SlotQuit");
    }
  }
}

//----------------------------------------------------------------------------------------
//
std::weak_ptr<CProjectEventCallbackRegistry> CSceneMainScreen::EventCallbackRegistry()
{
  return m_spEventCallbackRegistry;
}

//----------------------------------------------------------------------------------------
//
std::weak_ptr<CProjectRunner> CSceneMainScreen::ProjectRunner()
{
  return m_spProjectRunner;
}

//----------------------------------------------------------------------------------------
//
std::weak_ptr<CScriptRunner> CSceneMainScreen::ScriptRunner()
{
  return m_spScriptRunner;
}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::UnloadProject()
{
  if (!m_bInitialized) { return; }

  m_bShuttingDown = true;
  m_spScriptRunner->InterruptExecution();
}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::SetDebugging(bool bDebugging)
{
  if (m_bBeingDebugged != bDebugging)
  {
    m_bBeingDebugged = bDebugging;

    m_spUi->pQmlWidget->setAttribute(Qt::WA_AlwaysStackOnTop, !m_bBeingDebugged);
    m_spUi->pQmlWidget->setAttribute(Qt::WA_TranslucentBackground, !m_bBeingDebugged);

    // set size properties manually setzen, since this isn't done automatically
    QQuickItem* pRootObject =  m_spUi->pQmlWidget->rootObject();
    if (nullptr != pRootObject)
    {
      pRootObject->setProperty("debug", QVariant::fromValue(bDebugging));
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::SlotFinish()
{
  if (!m_bInitialized || nullptr == m_spCurrentProject) { return; }

  // no signal is giong to be emitted
  auto spSignalEmmiterContext = m_spScriptRunner->SignalEmmitterContext();
  if (nullptr != spSignalEmmiterContext &&
      CScriptRunnerSignalEmiter::ScriptExecStatus::eStopped ==
      spSignalEmmiterContext->ScriptExecutionStatus())
  {
    UnloadProject();

    emit SignalExitClicked();

    bool bOk =
        QMetaObject::invokeMethod(this, "SlotScriptRunFinished", Qt::QueuedConnection,
                                  Q_ARG(bool, false), Q_ARG(QString, QString()));
    assert(bOk);
    Q_UNUSED(bOk);
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::SlotQuit()
{
  if (!m_bInitialized || nullptr == m_spCurrentProject) { return; }

  CScriptRunnerSignalEmiter::ScriptExecStatus statusBefore =
      CScriptRunnerSignalEmiter::ScriptExecStatus::eStopped;
  auto spSignalEmmiterContext = m_spScriptRunner->SignalEmmitterContext();
  if (nullptr != spSignalEmmiterContext)
  {
    statusBefore = spSignalEmmiterContext->ScriptExecutionStatus();
    spSignalEmmiterContext->SetScriptExecutionStatus(CScriptRunnerSignalEmiter::eStopped);
    emit spSignalEmmiterContext->interrupt();
  }

  UnloadProject();

  emit SignalExitClicked();

  // no signal is giong to be emitted
  if (CScriptRunnerSignalEmiter::ScriptExecStatus::eStopped == statusBefore || m_bErrorState)
  {
    bool bOk =
        QMetaObject::invokeMethod(this, "SlotScriptRunFinished", Qt::QueuedConnection,
                                  Q_ARG(bool, false), Q_ARG(QString, QString()));
    assert(bOk);
    Q_UNUSED(bOk);
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::resizeEvent(QResizeEvent* pEvent)
{
  if (nullptr != pEvent)
  {
    QMetaObject::invokeMethod(this, "SlotResizeDone", Qt::QueuedConnection);
  }
  pEvent->accept();
}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::on_pQmlWidget_statusChanged(QQuickWidget::Status status)
{
  if (QQuickWidget::Error == status)
  {
    QStringList errors;
    const auto widgetErrors = m_spUi->pQmlWidget->errors();
    for (const QQmlError &error : widgetErrors)
    {
      errors.append(error.toString());
    }
    QString sErrors = errors.join(QStringLiteral(", "));
    qWarning() << sErrors;
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::on_pQmlWidget_sceneGraphError(QQuickWindow::SceneGraphError /*error*/,
                                                     const QString &message)
{
  qWarning() << message;
}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::SlotApplicationStateChanged(Qt::ApplicationState state)
{
  const bool bPauseWhenInactive = m_spSettings->PauseWhenInactive();
  if (state != Qt::ApplicationActive && bPauseWhenInactive)
  {
    auto spSignalEmmiterContext = m_spScriptRunner->SignalEmmitterContext();
    if (nullptr != spSignalEmmiterContext)
    {
      m_lastScriptExecutionStatus =
          static_cast<qint32>(spSignalEmmiterContext->ScriptExecutionStatus());
    }

    m_spScriptRunner->PauseExecution();
  }
  else if (state == Qt::ApplicationActive && bPauseWhenInactive)
  {
    if (m_lastScriptExecutionStatus ==
        static_cast<qint32>(CScriptRunnerSignalEmiter::ScriptExecStatus::eRunning))
    {
      m_spScriptRunner->ResumeExecution();
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::SlotError(QString sError, QtMsgType type)
{
  if (!m_bInitialized || nullptr == m_spCurrentProject) { return; }

  m_bErrorState = true;

  std::vector<QColor> vBgColors;
  std::vector<QColor> vTextColors;

  QString sPrefix;
  switch(type)
  {
  case QtMsgType::QtInfoMsg:
  case QtMsgType::QtDebugMsg:
    sPrefix = tr("Info");
    vTextColors.push_back({QColor(WHITE)});
    vBgColors.push_back({QColor(BLACK)});
    break;
  case QtMsgType::QtWarningMsg:
    sPrefix = tr("Warning");
    vTextColors.push_back({QColor(YELLOW)});
    vBgColors.push_back({QColor(BLACK)});
    break;
  case QtMsgType::QtCriticalMsg:
  case QtMsgType::QtFatalMsg:
    sPrefix = tr("Error");
    vTextColors.push_back({QColor(RED)});
    vBgColors.push_back({QColor(BLACK)});
    break;
  }

  QQuickItem* pRootObject =  m_spUi->pQmlWidget->rootObject();
  QMetaObject::invokeMethod(pRootObject, "showText",
                            Q_ARG(QVariant, QVariant(QString("%1: %2").arg(sPrefix).arg(sError))),
                            Q_ARG(QVariant, QVariant::fromValue(vTextColors)),
                            Q_ARG(QVariant, QVariant::fromValue(vBgColors)));
}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::SlotExecutionError(QString sException, qint32 iLine, QString sStack)
{
  if (!m_bInitialized || nullptr == m_spCurrentProject) { return; }

  SlotError(QString(tr("%1 on line %2 (%3)")).arg(sException).arg(iLine).arg(sStack), QtMsgType::QtCriticalMsg);
}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::SlotNextSkript(bool bMightBeRegex)
{
  if (!m_bInitialized || nullptr == m_spCurrentProject) { return; }

  m_spProjectRunner->ResolveScenes();
  NextSkript(bMightBeRegex);
}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::SlotResizeDone()
{
  // set size properties manually setzen, since this isn't done automatically
  QQuickItem* pRootObject =  m_spUi->pQmlWidget->rootObject();
  QSize newSize = size() -
      QSize(contentsMargins().left() +
            contentsMargins().right(),
            contentsMargins().top() +
            contentsMargins().bottom());
  if (nullptr != pRootObject)
  {
    pRootObject->setProperty("width", QVariant::fromValue(newSize.width()));
    pRootObject->setProperty("height",QVariant::fromValue(newSize.height()));
    QMetaObject::invokeMethod(pRootObject, "onResize");
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::SlotSceneSelectReturnValue(int iIndex)
{
  if (!m_bInitialized || nullptr == m_spCurrentProject) { return; }
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr == spDbManager) { return; }

  QStringList sScenes = m_spProjectRunner->PossibleScenes();
  if (0 <= iIndex && sScenes.size() > iIndex)
  {
    tspScene spScene = m_spProjectRunner->NextScene(sScenes[iIndex]);
    if (nullptr != spScene)
    {
      // load script
      QReadLocker lockerScene(&spScene->m_rwLock);
      QString sScript = spScene->m_sScript;
      lockerScene.unlock();
      bool bOk = QMetaObject::invokeMethod(m_spScriptRunner.get(), "LoadScript", Qt::QueuedConnection,
                                           Q_ARG(tspScene, spScene),
                                           Q_ARG(tspResource, spDbManager->FindResourceInProject(m_spCurrentProject, sScript)));
      assert(bOk);
      if (!bOk)
      {
        qWarning() << tr("LoadScript could not be called.");
        SlotQuit();
      }
    }
    else
    {
      qInfo() << tr("Next scene is null or end.");
      SlotFinish();
    }
  }
  else
  {
    qWarning() << tr("No more scenes to load was unexpected.");
    SlotFinish();
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::SlotScriptRunFinished(bool bOk, const QString& sRetVal)
{
  if (!m_bInitialized || nullptr == m_spCurrentProject) { return; }

  // normal handling
  if (!m_bShuttingDown)
  {
    if (bOk)
    {
      bool bMightBeRegex = CProjectRunner::MightBeRegexScene(sRetVal);
      if (sRetVal.isNull() || sRetVal.isEmpty())
      {
        SlotNextSkript(bMightBeRegex);
      }
      else
      {
        m_spProjectRunner->ResolveFindScenes(sRetVal);
        NextSkript(bMightBeRegex);
      }
    }
    else
    {
      SlotQuit();
    }
  }

  // during shutdown this slot is called, when the engine has unloaded safely
  else
  {
    DisconnectAllSignals();
    UnloadRunner();
    UnloadQml();
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::SlotStartLoadingSkript()
{
  NextSkript(false);
}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::SlotUnloadFinished()
{
  m_spEventCallbackRegistry->Clear();

  // disconnect this after unloading
  QQuickItem* pRootObject =  m_spUi->pQmlWidget->rootObject();
  disconnect(pRootObject, SIGNAL(unloadFinished()),
             this, SLOT(SlotUnloadFinished()));

  m_spUi->pQmlWidget->engine()->clearComponentCache();
  m_spUi->pQmlWidget->engine()->collectGarbage();
  m_spUi->pQmlWidget->setSource(QUrl());

  delete m_pCurrentProjectWrapper;
  if (!m_bBeingDebugged)
  {
    bool bOk = CDatabaseManager::UnloadProject(m_spCurrentProject);
    if (!bOk)
    {
      assert(bOk && "Failed to unload project");
      qWarning() << "Failed to unload project";
    }
  }
  m_spCurrentProject = nullptr;

  if (nullptr != m_spWindowContext && !m_bBeingDebugged)
  {
    m_spWindowContext->SignalChangeAppOverlay(QString());
  }

  emit SignalUnloadFinished();
  if (m_bCloseRequested)
  {
    qApp->quit();
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::ConnectAllSignals()
{
  QQuickItem* pRootObject =  m_spUi->pQmlWidget->rootObject();
  bool bOk = true;
  Q_UNUSED(bOk)

  bOk = connect(pRootObject, SIGNAL(startLoadingSkript()), this, SLOT(SlotStartLoadingSkript()));
  assert(bOk);
  bOk = connect(pRootObject, SIGNAL(quit()), this, SLOT(SlotQuit()));
  assert(bOk);

  auto spSignalEmmiterContext = m_spScriptRunner->SignalEmmitterContext();

  connect(spSignalEmmiterContext.get(), &CScriptRunnerSignalContext::showError,
          this, &CSceneMainScreen::SlotError, Qt::QueuedConnection);
  connect(m_spScriptRunner.get(), &CScriptRunner::SignalScriptRunFinished,
          this, &CSceneMainScreen::SlotScriptRunFinished, Qt::QueuedConnection);

  bOk = connect(pRootObject, SIGNAL(sceneSelectionReturnValue(int)),
                this, SLOT(SlotSceneSelectReturnValue(int)), Qt::QueuedConnection);
  assert(bOk);
  bOk = connect(pRootObject, SIGNAL(unloadFinished()),
                this, SLOT(SlotUnloadFinished()), Qt::QueuedConnection);
  assert(bOk);
}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::DisconnectAllSignals()
{
  QQuickItem* pRootObject =  m_spUi->pQmlWidget->rootObject();
  disconnect(pRootObject, SIGNAL(startLoadingSkript()), this, SLOT(SlotStartLoadingSkript()));
  disconnect(pRootObject, SIGNAL(quit()), this, SLOT(SlotQuit()));

  auto spSignalEmmiterContext = m_spScriptRunner->SignalEmmitterContext();

  disconnect(spSignalEmmiterContext.get(), &CScriptRunnerSignalContext::showError,
          this, &CSceneMainScreen::SlotError);
  disconnect(m_spScriptRunner.get(), &CScriptRunner::SignalScriptRunFinished,
          this, &CSceneMainScreen::SlotScriptRunFinished);

  disconnect(pRootObject, SIGNAL(sceneSelectionReturnValue(int)),
             this, SLOT(SlotSceneSelectReturnValue(int)));
}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::InitQmlMain()
{
  m_spUi->pQmlWidget->setAttribute(Qt::WA_AlwaysStackOnTop, true);
  m_spUi->pQmlWidget->setAttribute(Qt::WA_TranslucentBackground, true);
  m_spUi->pQmlWidget->setClearColor(Qt::transparent);
  m_spUi->pQmlWidget->setStyleSheet("background-color: transparent;");

  xmldom::RegisterWrapper(m_spUi->pQmlWidget->engine());

  m_spUi->pQmlWidget->engine()->setProperty(player::c_sMainPlayerProperty, QVariant::fromValue(this));

  QQmlEngine::setObjectOwnership(m_spUi->pQmlWidget->engine(), QQmlEngine::CppOwnership);
  // engine will allways take owership of this object
  CDatabaseImageProvider* pProvider = new CDatabaseImageProvider(m_wpDbManager);
  m_spUi->pQmlWidget->engine()->addImageProvider("DataBaseImageProivider", pProvider);

  // for eos compatibility
  QJSValue jsSceneManagerMetaObject = m_spUi->pQmlWidget->engine()->newQMetaObject(&CProjectEventWrapper::staticMetaObject);
  m_spUi->pQmlWidget->engine()->globalObject().setProperty("Event", jsSceneManagerMetaObject);
}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::LoadQml()
{
  QQuickItem* pRootObject =  m_spUi->pQmlWidget->rootObject();
  if (nullptr != pRootObject)
  {
    m_pCurrentProjectWrapper = new CProjectScriptWrapper(m_spUi->pQmlWidget->engine(), m_spCurrentProject);
    QQmlEngine::setObjectOwnership(m_pCurrentProjectWrapper, QQmlEngine::CppOwnership);
    pRootObject->setProperty("currentlyLoadedProject", QVariant::fromValue(m_pCurrentProjectWrapper.data()));

    QMetaObject::invokeMethod(pRootObject, "onLoadProject");

    pRootObject->setProperty("debug", m_bBeingDebugged);
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::NextSkript(bool bMightBeRegex)
{
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr == spDbManager) { return; }

  QQuickItem* pRootObject =  m_spUi->pQmlWidget->rootObject();
  QMetaObject::invokeMethod(pRootObject, "clearTextBox");

  QStringList sScenes = m_spProjectRunner->PossibleScenes();
  if (sScenes.size() > 0)
  {
    if (sScenes.size() == 1)
    {
      tspScene spScene = m_spProjectRunner->NextScene(sScenes[0]);
      if (nullptr != spScene)
      {
        // load script
        QReadLocker lockerScene(&spScene->m_rwLock);
        QString sScript = spScene->m_sScript;
        lockerScene.unlock();
        bool bOk = QMetaObject::invokeMethod(m_spScriptRunner.get(), "LoadScript", Qt::QueuedConnection,
                                             Q_ARG(tspScene, spScene),
                                             Q_ARG(tspResource, spDbManager->FindResourceInProject(m_spCurrentProject, sScript)));
        assert(bOk);
        if (!bOk)
        {
          qWarning() << tr("LoadScript could not be called.");
          SlotQuit();
        }
      }
      else
      {
        qInfo() << tr("Next scene is null or end.");
        SlotFinish();
      }
    }
    else if (bMightBeRegex)
    {
      long unsigned int seed =
        static_cast<long unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
      std::mt19937 generator(static_cast<long unsigned int>(seed));
      std::uniform_int_distribution<> dis(0, static_cast<qint32>(sScenes.size() - 1));
      qint32 iGeneratedIndex = dis(generator);
      tspScene spScene = m_spProjectRunner->NextScene(sScenes[iGeneratedIndex]);
      if (nullptr != spScene)
      {
        // load script
        QReadLocker lockerScene(&spScene->m_rwLock);
        QString sScript = spScene->m_sScript;
        lockerScene.unlock();
        bool bOk = QMetaObject::invokeMethod(m_spScriptRunner.get(), "LoadScript", Qt::QueuedConnection,
                                             Q_ARG(tspScene, spScene),
                                             Q_ARG(tspResource, spDbManager->FindResourceInProject(m_spCurrentProject, sScript)));
        assert(bOk);
        if (!bOk)
        {
          qWarning() << tr("LoadScript could not be called.");
          SlotQuit();
        }
      }
      else
      {
        qInfo() << tr("Next scene is null or end.");
        SlotFinish();
      }
    }
    else
    {
      bool bOk = QMetaObject::invokeMethod(pRootObject, "showSceneSelection",
                                           Q_ARG(QVariant, sScenes));
      assert(bOk);
      if (!bOk)
      {
        qWarning() << tr("showSceneSelection could not be called.");
        SlotQuit();
      }
    }
  }
  else
  {
    qInfo() << tr("No more scenes to load.");
    SlotFinish();
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::UnloadRunner()
{
  m_spScriptRunner->UnregisterComponents();
  m_spProjectRunner->UnloadProject();
}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::UnloadQml()
{
  QQuickItem* pRootObject =  m_spUi->pQmlWidget->rootObject();
  if (nullptr != pRootObject)
  {
    QMetaObject::invokeMethod(pRootObject, "onUnLoadProject");
  }
}

//----------------------------------------------------------------------------------------
//
CSceneMainScreenWrapper::CSceneMainScreenWrapper(QObject* pParent,
                                                 QPointer<CSceneMainScreen> pPlayer) :
  QObject(pParent),
  m_pPlayer(pPlayer)
{
}
CSceneMainScreenWrapper::~CSceneMainScreenWrapper()
{

}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreenWrapper::initObject(QJSValue wrapper)
{
  if (wrapper.isObject() && nullptr != m_pPlayer)
  {
    CProjectEventTargetWrapper* pObject =
        qobject_cast<CProjectEventTargetWrapper*>(wrapper.toQObject());
    if (nullptr != pObject)
    {
      CProjectSceneManagerWrapper* pSceneManager =
          qobject_cast<CProjectSceneManagerWrapper*>(pObject);
      if (nullptr != pSceneManager)
      {
        pSceneManager->Initalize(m_pPlayer->ProjectRunner(), m_pPlayer->EventCallbackRegistry());
      }
    }
  }
}
