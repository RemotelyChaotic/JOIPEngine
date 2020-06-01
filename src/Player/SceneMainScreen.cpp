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
#include "Widgets/BackgroundWidget.h"
#include "ui_SceneMainScreen.h"

#include <QAction>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QResizeEvent>
#include <QUrl>

#include <assert.h>

CSceneMainScreen::CSceneMainScreen(QWidget* pParent) :
  QWidget(pParent),
  m_spUi(std::make_unique<Ui::CSceneMainScreen>()),
  m_spProjectRunner(std::make_unique<CProjectRunner>()),
  m_spSettings(CApplication::Instance()->Settings()),
  m_spCurrentProject(nullptr),
  m_pCurrentProjectWrapper(nullptr),
  m_wpDbManager(),
  m_wpScriptRunner(),
  m_bInitialized(false)
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

  m_wpDbManager = CApplication::Instance()->System<CDatabaseManager>();
  m_wpScriptRunner = CApplication::Instance()->System<CScriptRunner>();

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
    m_spProjectRunner->LoadProject(m_spCurrentProject, sStartScene);

    m_pCurrentProjectWrapper = new CProject(m_spUi->pQmlWidget->engine(), m_spCurrentProject);
    QQuickItem* pRootObject =  m_spUi->pQmlWidget->rootObject();
    pRootObject->setProperty("currentlyLoadedProject", QVariant::fromValue(m_pCurrentProjectWrapper.data()));

    ConnectAllSignals();

    QMetaObject::invokeMethod(pRootObject, "onLoadProject");
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::UnloadProject()
{
  if (!m_bInitialized) { return; }

  auto spScriptRunner = m_wpScriptRunner.lock();
  if (nullptr != spScriptRunner)
  {
    auto spSignalEmmiterContext = spScriptRunner->SignalEmmitterContext();
    if (nullptr != spSignalEmmiterContext)
    {
      emit spSignalEmmiterContext->clearStorage();
    }

    disconnect(spScriptRunner.get(), &CScriptRunner::SignalScriptRunFinished,
            this, &CSceneMainScreen::SlotScriptRunFinished);

    spScriptRunner->unregisterComponents();
  }

  m_spProjectRunner->UnloadProject();

  QQuickItem* pRootObject =  m_spUi->pQmlWidget->rootObject();
  QMetaObject::invokeMethod(pRootObject, "onUnLoadProject");

  m_spUi->pQmlWidget->engine()->clearComponentCache();
  m_spUi->pQmlWidget->engine()->collectGarbage();


  delete m_pCurrentProjectWrapper;
  m_spCurrentProject = nullptr;


  DisconnectAllSignals();
}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::SlotQuit()
{
  if (!m_bInitialized || nullptr == m_spCurrentProject) { return; }

  UnloadProject();
  auto spScriptRunner = m_wpScriptRunner.lock();
  if (nullptr != spScriptRunner)
  {
    auto spSignalEmmiterContext = spScriptRunner->SignalEmmitterContext();
    if (nullptr != spSignalEmmiterContext)
    {
      spSignalEmmiterContext->SetScriptExecutionStatus(CScriptRunnerSignalEmiter::eStopped);
      emit spSignalEmmiterContext->interrupt();
    }
  }
  emit SignalExitClicked();
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
    vBgColors.push_back({QColor(RED)});
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
      auto spScriptRunner = m_wpScriptRunner.lock();
      if (nullptr != spScriptRunner && nullptr != spDbManager)
      {
        QReadLocker lockerScene(&spScene->m_rwLock);
        QString sScript = spScene->m_sScript;
        lockerScene.unlock();
        QMetaObject::invokeMethod(spScriptRunner.get(), "loadScript", Qt::QueuedConnection,
                                  Q_ARG(tspScene, spScene),
                                  Q_ARG(tspResource, spDbManager->FindResourceInProject(m_spCurrentProject, sScript)));
      }
      else
      {
        qWarning() << tr("Script-Runner or Database-Manager missing.");
        SlotQuit();
      }
    }
    else
    {
      qInfo() << tr("Next scene is null or end.");
      SlotQuit();
    }
  }
  else
  {
    qWarning() << tr("No more scenes to load was unexpected.");
    SlotQuit();
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::SlotScriptRunFinished(bool bOk, const QString& sRetVal)
{
  if (!m_bInitialized || nullptr == m_spCurrentProject) { return; }

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
    qWarning() << tr("Error in script, unloading project.");
    SlotQuit();
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
void CSceneMainScreen::ConnectAllSignals()
{
  QQuickItem* pRootObject =  m_spUi->pQmlWidget->rootObject();
  connect(pRootObject, SIGNAL(startLoadingSkript()), this, SLOT(SlotStartLoadingSkript()));
  connect(pRootObject, SIGNAL(quit()), this, SLOT(SlotQuit()));

  auto spScriptRunner = m_wpScriptRunner.lock();
  if (nullptr != spScriptRunner)
  {
    auto spSignalEmmiterContext = spScriptRunner->SignalEmmitterContext();

    connect(spSignalEmmiterContext.get(), &CScriptRunnerSignalContext::showError,
            this, &CSceneMainScreen::SlotError, Qt::QueuedConnection);
    connect(spSignalEmmiterContext.get(), &CScriptRunnerSignalContext::executionError,
            this, &CSceneMainScreen::SlotExecutionError, Qt::QueuedConnection);
    connect(spScriptRunner.get(), &CScriptRunner::SignalScriptRunFinished,
            this, &CSceneMainScreen::SlotScriptRunFinished, Qt::QueuedConnection);

    bool bOk = true;
    Q_UNUSED(bOk)

    bOk = connect(pRootObject, SIGNAL(sceneSelectionReturnValue(int)),
                  this, SLOT(SlotSceneSelectReturnValue(int)), Qt::QueuedConnection);
    assert(bOk);
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::DisconnectAllSignals()
{
  QQuickItem* pRootObject =  m_spUi->pQmlWidget->rootObject();
  disconnect(pRootObject, SIGNAL(startLoadingSkript()), this, SLOT(SlotStartLoadingSkript()));
  disconnect(pRootObject, SIGNAL(quit()), this, SLOT(SlotQuit()));

  auto spScriptRunner = m_wpScriptRunner.lock();
  if (nullptr != spScriptRunner)
  {
    auto spSignalEmmiterContext = spScriptRunner->SignalEmmitterContext();

    disconnect(spSignalEmmiterContext.get(), &CScriptRunnerSignalContext::showError,
            this, &CSceneMainScreen::SlotError);
    disconnect(spSignalEmmiterContext.get(), &CScriptRunnerSignalContext::executionError,
            this, &CSceneMainScreen::SlotExecutionError);
    disconnect(spScriptRunner.get(), &CScriptRunner::SignalScriptRunFinished,
            this, &CSceneMainScreen::SlotScriptRunFinished);

    disconnect(pRootObject, SIGNAL(sceneSelectionReturnValue(int)),
               this, SLOT(SlotSceneSelectReturnValue(int)));
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::InitQmlMain()
{
  m_spUi->pQmlWidget->setAttribute(Qt::WA_AlwaysStackOnTop, true);
  m_spUi->pQmlWidget->setAttribute(Qt::WA_TranslucentBackground, true);
  m_spUi->pQmlWidget->setClearColor(Qt::transparent);
  m_spUi->pQmlWidget->setStyleSheet("background-color: transparent;");

  QQmlEngine::setObjectOwnership(m_spUi->pQmlWidget->engine(), QQmlEngine::CppOwnership);
  // engine will allways take owership of this object
  CDatabaseImageProvider* pProvider = new CDatabaseImageProvider(m_wpDbManager);
  m_spUi->pQmlWidget->engine()->addImageProvider("DataBaseImageProivider", pProvider);
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
        auto spScriptRunner = m_wpScriptRunner.lock();
        if (nullptr != spScriptRunner && nullptr != spDbManager)
        {
          QReadLocker lockerScene(&spScene->m_rwLock);
          QString sScript = spScene->m_sScript;
          lockerScene.unlock();
          bool bOk = QMetaObject::invokeMethod(spScriptRunner.get(), "loadScript", Qt::QueuedConnection,
                                               Q_ARG(tspScene, spScene),
                                               Q_ARG(tspResource, spDbManager->FindResourceInProject(m_spCurrentProject, sScript)));
          assert(bOk);
          if (!bOk)
          {
            qWarning() << tr("loadScript could not be called.");
          }
        }
        else
        {
          qWarning() << tr("Script-Runner or Database-Manager missing.");
          SlotQuit();
        }
      }
      else
      {
        qInfo() << tr("Next scene is null or end.");
        SlotQuit();
      }
    }
    else
    {
      QMetaObject::invokeMethod(pRootObject, "showSceneSelection",
                                Q_ARG(QVariant, sScenes));
    }
  }
  else
  {
    qInfo() << tr("No more scenes to load.");
    SlotQuit();
  }
}
