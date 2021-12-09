#include "SceneMainScreen.h"
#include "Application.h"
#include "Constants.h"
#include "ProjectRunner.h"
#include "Settings.h"
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

const char* player::c_sMainPlayerProperty = "MainPlayer";

//----------------------------------------------------------------------------------------
//
CSceneMainScreen::CSceneMainScreen(QWidget* pParent) :
  QWidget(pParent),
  m_spUi(std::make_unique<Ui::CSceneMainScreen>()),
  m_spProjectRunner(std::make_unique<CProjectRunner>()),
  m_spScriptRunnerSystem(std::make_shared<CThreadedSystem>("ScriptRunner")),
  m_spScriptRunner(nullptr),
  m_spSettings(CApplication::Instance()->Settings()),
  m_spCurrentProject(nullptr),
  m_pCurrentProjectWrapper(nullptr),
  m_wpDbManager(),
  m_lastScriptExecutionStatus(static_cast<qint32>(CScriptRunnerSignalEmiter::ScriptExecStatus::eRunning)),
  m_bInitialized(false),
  m_bShuttingDown(false),
  m_bBeingDebugged(false)
{
  m_spUi->setupUi(this);

  Initialize();
}

CSceneMainScreen::~CSceneMainScreen()
{
}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::Initialize()
{
  m_bInitialized = false;

  connect(CApplication::Instance(), &QGuiApplication::applicationStateChanged,
          this, &CSceneMainScreen::SlotApplicationStateChanged);

  m_spScriptRunnerSystem->RegisterObject<CScriptRunner>();
  m_spScriptRunner = std::dynamic_pointer_cast<CScriptRunner>(m_spScriptRunnerSystem->Get());
  assert(m_spScriptRunner != nullptr);
  QQmlEngine::setObjectOwnership(m_spScriptRunner.get(), QQmlEngine::CppOwnership);

  m_wpDbManager = CApplication::Instance()->System<CDatabaseManager>();

  InitQmlMain();

  connect(m_spProjectRunner.get(), &CProjectRunner::SignalError,
          this, &CSceneMainScreen::SlotError);

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

    ConnectAllSignals();
    LoadQml();

    m_bShuttingDown = false;
  }
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
  if (CScriptRunnerSignalEmiter::ScriptExecStatus::eStopped == statusBefore)
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
void CSceneMainScreen::SlotNextSkript()
{
  if (!m_bInitialized || nullptr == m_spCurrentProject) { return; }

  m_spProjectRunner->ResolveScenes();
  NextSkript();
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
      if (sRetVal.isNull() || sRetVal.isEmpty())
      {
        SlotNextSkript();
      }
      else
      {
        m_spProjectRunner->ResolveFindScenes(sRetVal);
        NextSkript();
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

    disconnect(m_spScriptRunner.get(), &CScriptRunner::SignalScriptRunFinished,
            this, &CSceneMainScreen::SlotScriptRunFinished);

    bool bOk = QMetaObject::invokeMethod(this, "SlotUnloadFinished", Qt::QueuedConnection);
    assert(bOk); Q_UNUSED(bOk)
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::SlotStartLoadingSkript()
{
  NextSkript();
}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::SlotUnloadFinished()
{
  m_spUi->pQmlWidget->engine()->clearComponentCache();
  m_spUi->pQmlWidget->engine()->collectGarbage();
  m_spUi->pQmlWidget->setSource(QUrl());

  delete m_pCurrentProjectWrapper;
  m_spCurrentProject = nullptr;

  CDatabaseManager::UnloadProject(m_spCurrentProject);
  m_spCurrentProject = nullptr;

  emit SignalUnloadFinished();
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
void CSceneMainScreen::NextSkript()
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

