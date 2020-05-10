#include "SceneMainScreen.h"
#include "Application.h"
#include "BackgroundWidget.h"
#include "Constants.h"
#include "ProjectRunner.h"
#include "Settings.h"
#include "Systems/DatabaseManager.h"
#include "Systems/DatabaseImageProvider.h"
#include "Systems/Project.h"
#include "Systems/ScriptRunner.h"
#include "Systems/Script/ScriptRunnerSignalEmiter.h"
#include "ui_SceneMainScreen.h"

#include <QAction>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QResizeEvent>
#include <QUrl>

#include <assert.h>

#undef FindResourceInProject

CSceneMainScreen::CSceneMainScreen(QWidget* pParent) :
  QWidget(pParent),
  m_spUi(std::make_unique<Ui::CSceneMainScreen>()),
  m_spProjectRunner(std::make_unique<CProjectRunner>()),
  m_spSettings(CApplication::Instance()->Settings()),
  m_spCurrentProject(nullptr),
  m_pCurrentProjectWrapper(nullptr),
  m_wpDbManager(),
  m_wpScriptRunner(),
  m_pActionSkip(new QAction(this)),
  m_pActionQuit(new QAction(this)),
  m_bInitialized(false)
{
  m_spUi->setupUi(this);

  m_pActionSkip->setShortcuts(KEY_COMMIT);
  m_pActionQuit->setShortcuts(KEY_ESCAPE);

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

  // keyboard actions
  //connect(m_pActionSkip, &QAction::triggered,
  //        m_spUi->pInfoDisplay, &CInformationWidget::SlotSkipTimeout);
  //connect(m_pActionSkip, &QAction::triggered,
  //        m_spUi->pInfoDisplay, &CInformationWidget::SignalWaitSkipped, Qt::QueuedConnection);
  //connect(m_pActionQuit, &QAction::triggered, this, &CSceneMainScreen::SlotQuit);
  //
  //m_spUi->pInfoDisplay->Initialize();
  //m_spUi->pTextBoxDisplay->Initialize();
  //m_spUi->pTimerDisplay->Initialize();
  //
  //m_spUi->pResourceDisplay->setFixedSize(width() * 2 / 4, height() * 2 / 3);
  //m_spUi->pResourceDisplay->SetMargins(10, 10, 10, 10);
  //m_spUi->pSoundEmmiter->setVisible(false);
  //
  //QGraphicsDropShadowEffect* pShadow = new QGraphicsDropShadowEffect(m_spUi->pResourceDisplay);
  //pShadow->setBlurRadius(5);
  //pShadow->setXOffset(5);
  //pShadow->setYOffset(5);
  //pShadow->setColor(Qt::black);
  //m_spUi->pResourceDisplay->setGraphicsEffect(pShadow);
  //
  //// signals from widgets
  //connect(m_pBackground, &CBackgroundWidget::SignalError,
  //        this, &CSceneMainScreen::SlotError);
  //connect(m_spUi->pInfoDisplay, &CInformationWidget::SignalError,
  //        this, &CSceneMainScreen::SlotError);
  //connect(m_spUi->pInfoDisplay, &CInformationWidget::SignalQuit,
  //        this, &CSceneMainScreen::SlotQuit);
  //connect(m_spProjectRunner.get(), &CProjectRunner::SignalError,
  //        this, &CSceneMainScreen::SlotError);
  //connect(m_spUi->pTextBoxDisplay, &CTextBoxWidget::SignalShowSceneSelectReturnValue,
  //        this, &CSceneMainScreen::SlotSceneSelectReturnValue);
  //connect(m_spUi->pResourceDisplay, &CResourceDisplayWidget::SignalLoadFinished,
  //        this, &CSceneMainScreen::SlotLoadFinished);
  //
  //// filter for skipping text
  //m_pBackground->installEventFilter(m_spUi->pInfoDisplay);
  //m_spUi->pResourceDisplay->installEventFilter(m_spUi->pInfoDisplay);
  //m_spUi->pTextBoxDisplay->installEventFilter(m_spUi->pInfoDisplay);

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

    //m_spCurrentProject->m_rwLock.lockForRead();
    //QString sTitle = m_spCurrentProject->m_sTitleCard;
    //m_spCurrentProject->m_rwLock.unlock();

    //tspResource spTitle = spDbManager->FindResourceInProject(m_spCurrentProject, sTitle);
    //m_spUi->pResourceDisplay->UnloadResource();
    //m_spUi->pResourceDisplay->LoadResource(spTitle);
  }

  //m_spUi->pTextBoxDisplay->SetSceneSelection(false);
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

  //m_spUi->pInfoDisplay->SlotHideIcon(QString());
  //m_spUi->pTextBoxDisplay->SlotClearText();
  //m_spUi->pTimerDisplay->SlotStopTimer();
  //m_spUi->pTimerDisplay->SlotHideTimer();

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

  //m_spUi->pResourceDisplay->setFixedSize(pEvent->size().width() * 2 / 4,
  //                                       pEvent->size().height() * 2 / 3);
  //
  //m_pBackground->setFixedSize(pEvent->size());
  //m_pBackground->setGeometry(0, 0, pEvent->size().width(), pEvent->size().height());
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

  //std::vector<QColor> vBgColors = m_spUi->pTextBoxDisplay->BackgroundColors();
  //std::vector<QColor> vTextColors = m_spUi->pTextBoxDisplay->TextColors();
  //
  //QString sPrefix;
  //switch(type)
  //{
  //case QtMsgType::QtInfoMsg:
  //case QtMsgType::QtDebugMsg:
  //  sPrefix = tr("Info");
  //  m_spUi->pTextBoxDisplay->SlotTextColorsChanged({QColor(WHITE)});
  //  m_spUi->pTextBoxDisplay->SlotTextBackgroundColorsChanged({QColor(BLACK)});
  //  break;
  //case QtMsgType::QtWarningMsg:
  //  sPrefix = tr("Warning");
  //  m_spUi->pTextBoxDisplay->SlotTextColorsChanged({QColor(YELLOW)});
  //  m_spUi->pTextBoxDisplay->SlotTextBackgroundColorsChanged({QColor(BLACK)});
  //  break;
  //case QtMsgType::QtCriticalMsg:
  //case QtMsgType::QtFatalMsg:
  //  sPrefix = tr("Error");
  //  m_spUi->pTextBoxDisplay->SlotTextColorsChanged({QColor(RED)});
  //  m_spUi->pTextBoxDisplay->SlotTextBackgroundColorsChanged({QColor(RED)});
  //  break;
  //}
  //
  //m_spUi->pTextBoxDisplay->SlotShowText(QString("%1: %2").arg(sPrefix).arg(sError));
  //
  //m_spUi->pTextBoxDisplay->SlotTextColorsChanged(vTextColors);
  //m_spUi->pTextBoxDisplay->SlotTextBackgroundColorsChanged(vBgColors);
}

//----------------------------------------------------------------------------------------
//
//void CSceneMainScreen::SlotLoadFinished()
//{
//  if (!m_bInitialized || nullptr == m_spCurrentProject) { return; }
//
//  m_spUi->pResourceDisplay->SlotSetSliderVisible(false);
//}

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
//void CSceneMainScreen::SlotPauseVideo()
//{
//  if (!m_bInitialized || nullptr == m_spCurrentProject) { return; }
//
//  if (m_spUi->pResourceDisplay->IsRunning())
//  {
//    m_spUi->pResourceDisplay->SlotPlayPause();
//  }
//}
//
////----------------------------------------------------------------------------------------
////
//void CSceneMainScreen::SlotPauseSound()
//{
//  if (!m_bInitialized || nullptr == m_spCurrentProject) { return; }
//
//  if (m_spUi->pSoundEmmiter->IsRunning())
//  {
//    m_spUi->pSoundEmmiter->SlotPlayPause();
//  }
//}

//----------------------------------------------------------------------------------------
//
//void CSceneMainScreen::SlotPlayMedia(tspResource spResource)
//{
//  if (!m_bInitialized || nullptr == m_spCurrentProject) { return; }
//
//  if (nullptr != spResource)
//  {
//    QReadLocker locker(&spResource->m_rwLock);
//    EResourceType type = spResource->m_type;
//    locker.unlock();
//    if (type._to_integral() == EResourceType::eSound)
//    {
//      m_spUi->pSoundEmmiter->UnloadResource();
//      m_spUi->pSoundEmmiter->LoadResource(spResource);
//    }
//    else if (type._to_integral() == EResourceType::eMovie ||
//             type._to_integral() == EResourceType::eImage)
//    {
//      SlotShowMedia(spResource);
//    }
//  }
//  else
//  {
//    if (!m_spUi->pSoundEmmiter->IsRunning())
//    {
//      m_spUi->pSoundEmmiter->SlotPlayPause();
//    }
//    if (!m_spUi->pResourceDisplay->IsRunning())
//    {
//      m_spUi->pResourceDisplay->SlotPlayPause();
//    }
//  }
//}

//----------------------------------------------------------------------------------------
//
//void CSceneMainScreen::SlotSceneSelectReturnValue(qint32 iIndex)
//{
//  if (!m_bInitialized || nullptr == m_spCurrentProject) { return; }
//  auto spDbManager = m_wpDbManager.lock();
//  if (nullptr == spDbManager) { return; }
//
//  QStringList sScenes = m_spProjectRunner->PossibleScenes();
//  if (0 <= iIndex && sScenes.size() > iIndex)
//  {
//    tspScene spScene = m_spProjectRunner->NextScene(sScenes[iIndex]);
//    if (nullptr != spScene)
//    {
//      // load script
//      auto spScriptRunner = m_wpScriptRunner.lock();
//      if (nullptr != spScriptRunner && nullptr != spDbManager)
//      {
//        QReadLocker lockerScene(&spScene->m_rwLock);
//        QString sScript = spScene->m_sScript;
//        lockerScene.unlock();
//        QMetaObject::invokeMethod(spScriptRunner.get(), "loadScript", Qt::QueuedConnection,
//                                  Q_ARG(tspScene, spScene),
//                                  Q_ARG(tspResource, spDbManager->FindResourceInProject(m_spCurrentProject, sScript)));
//      }
//      else
//      {
//        qWarning() << tr("Script-Runner or Database-Manager missing.");
//        SlotQuit();
//      }
//    }
//    else
//    {
//      qInfo() << tr("Next scene is null or end.");
//      SlotQuit();
//    }
//  }
//  else
//  {
//    qWarning() << tr("No more scenes to load was unexpected.");
//    SlotQuit();
//  }
//}

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
//void CSceneMainScreen::SlotShowMedia(tspResource spResource)
//{
//  if (!m_bInitialized || nullptr == m_spCurrentProject || nullptr == spResource) { return; }
//
//  m_spUi->pResourceDisplay->UnloadResource();
//  m_spUi->pResourceDisplay->SlotSetSliderVisible(false);
//  m_spUi->pResourceDisplay->LoadResource(spResource);
//}
//
////----------------------------------------------------------------------------------------
////
//void CSceneMainScreen::SlotStopVideo()
//{
//  if (!m_bInitialized || nullptr == m_spCurrentProject) { return; }
//
//  m_spUi->pResourceDisplay->SlotStop();
//}
//
////----------------------------------------------------------------------------------------
////
//void CSceneMainScreen::SlotStopSound()
//{
//  if (!m_bInitialized || nullptr == m_spCurrentProject) { return; }
//
//  m_spUi->pSoundEmmiter->SlotStop();
//}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::ConnectAllSignals()
{
  connect(m_spUi->pQmlWidget->rootObject(), SIGNAL(startLoadingSkript()),
          this, SLOT(SlotStartLoadingSkript()));

  auto spScriptRunner = m_wpScriptRunner.lock();
  if (nullptr != spScriptRunner)
  {
    auto spSignalEmmiterContext = spScriptRunner->SignalEmmitterContext();

    connect(spScriptRunner.get(), &CScriptRunner::SignalScriptRunFinished,
            this, &CSceneMainScreen::SlotScriptRunFinished, Qt::QueuedConnection);

    // Backgrounds
    //connect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalBackgroundColorChanged,
    //        m_pBackground, &CBackgroundWidget::SlotBackgroundColorChanged, Qt::QueuedConnection);
    //connect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalBackgroundTextureChanged,
    //        m_pBackground, &CBackgroundWidget::SlotBackgroundTextureChanged, Qt::QueuedConnection);
    //
    //// Icons
    //connect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalHideIcon,
    //        m_spUi->pInfoDisplay, &CInformationWidget::SlotHideIcon, Qt::QueuedConnection);
    //connect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalShowIcon,
    //        m_spUi->pInfoDisplay, &CInformationWidget::SlotShowIcon, Qt::QueuedConnection);
    //
    //// Media Player
    //connect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalPauseVideo,
    //        this, &CSceneMainScreen::SlotPauseVideo, Qt::QueuedConnection);
    //connect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalPauseSound,
    //        this, &CSceneMainScreen::SlotPauseSound, Qt::QueuedConnection);
    //connect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalPlayMedia,
    //        this, &CSceneMainScreen::SlotPlayMedia, Qt::QueuedConnection);
    //connect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalShowMedia,
    //        this, &CSceneMainScreen::SlotShowMedia, Qt::QueuedConnection);
    //connect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalStopVideo,
    //        this, &CSceneMainScreen::SlotStopVideo, Qt::QueuedConnection);
    //connect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalStopSound,
    //        this, &CSceneMainScreen::SlotStopSound, Qt::QueuedConnection);
    //connect(m_spUi->pResourceDisplay, &CResourceDisplayWidget::SignalPlaybackFinished,
    //        spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalPlaybackFinished, Qt::QueuedConnection);
    //
    //// TextBox
    //connect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalClearText,
    //        m_spUi->pTextBoxDisplay, &CTextBoxWidget::SlotClearText, Qt::QueuedConnection);
    //connect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalShowButtonPrompts,
    //        m_spUi->pTextBoxDisplay, &CTextBoxWidget::SlotShowButtonPrompts, Qt::QueuedConnection);
    //connect(m_spUi->pTextBoxDisplay, &CTextBoxWidget::SignalShowButtonReturnValue,
    //        spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalShowButtonReturnValue,
    //        Qt::QueuedConnection);
    //connect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalShowInput,
    //        m_spUi->pTextBoxDisplay, &CTextBoxWidget::SlotShowInput, Qt::QueuedConnection);
    //connect(m_spUi->pTextBoxDisplay, &CTextBoxWidget::SignalShowInputReturnValue,
    //        spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalShowInputReturnValue,
    //        Qt::QueuedConnection);
    //connect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalShowText,
    //        m_spUi->pTextBoxDisplay, &CTextBoxWidget::SlotShowText, Qt::QueuedConnection);
    //connect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalTextBackgroundColorsChanged,
    //        m_spUi->pTextBoxDisplay, &CTextBoxWidget::SlotTextBackgroundColorsChanged, Qt::QueuedConnection);
    //connect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalTextColorsChanged,
    //        m_spUi->pTextBoxDisplay, &CTextBoxWidget::SlotTextColorsChanged, Qt::QueuedConnection);
    //
    //// Timer
    //connect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalHideTimer,
    //        m_spUi->pTimerDisplay, &CTimerDisplayWidget::SlotHideTimer, Qt::QueuedConnection);
    //connect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalSetTime,
    //        m_spUi->pTimerDisplay, &CTimerDisplayWidget::SlotSetTime, Qt::QueuedConnection);
    //connect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalSetTimeVisible,
    //        m_spUi->pTimerDisplay, &CTimerDisplayWidget::SlotSetTimeVisible, Qt::QueuedConnection);
    //connect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalHideTimer,
    //        m_spUi->pTimerDisplay, &CTimerDisplayWidget::SlotHideTimer, Qt::QueuedConnection);
    //connect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalShowTimer,
    //        m_spUi->pTimerDisplay, &CTimerDisplayWidget::SlotShowTimer, Qt::QueuedConnection);
    //connect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalStartTimer,
    //        m_spUi->pTimerDisplay, &CTimerDisplayWidget::SlotStartTimer, Qt::QueuedConnection);
    //connect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalStopTimer,
    //        m_spUi->pTimerDisplay, &CTimerDisplayWidget::SlotStopTimer, Qt::QueuedConnection);
    //connect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalWaitForTimer,
    //        m_spUi->pTimerDisplay, &CTimerDisplayWidget::SlotWaitForTimer, Qt::QueuedConnection);
    //connect(m_spUi->pTimerDisplay, &CTimerDisplayWidget::SignalTimerFinished,
    //        spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalTimerFinished, Qt::QueuedConnection);
    //
    //// Thread
    //connect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalSkippableWait,
    //        m_spUi->pInfoDisplay, &CInformationWidget::SlotShowSkipIcon, Qt::QueuedConnection);
    //connect(m_spUi->pInfoDisplay, &CInformationWidget::SignalWaitSkipped,
    //        spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalWaitSkipped, Qt::QueuedConnection);
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::DisconnectAllSignals()
{
  disconnect(m_spUi->pQmlWidget->rootObject(), SIGNAL(startLoadingSkript()),
             this, SLOT(SlotStartLoadingSkript()));

  auto spScriptRunner = m_wpScriptRunner.lock();
  if (nullptr != spScriptRunner)
  {
    auto spSignalEmmiterContext = spScriptRunner->SignalEmmitterContext();

    disconnect(spScriptRunner.get(), &CScriptRunner::SignalScriptRunFinished,
            this, &CSceneMainScreen::SlotScriptRunFinished);

    // backgrounds
    //connect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalBackgroundColorChanged,
    //        m_pBackground, &CBackgroundWidget::SlotBackgroundColorChanged, Qt::QueuedConnection);
    //disconnect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalBackgroundTextureChanged,
    //        m_pBackground, &CBackgroundWidget::SlotBackgroundTextureChanged);
    //
    //// icons
    //disconnect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalHideIcon,
    //        m_spUi->pInfoDisplay, &CInformationWidget::SlotHideIcon);
    //disconnect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalShowIcon,
    //        m_spUi->pInfoDisplay, &CInformationWidget::SlotShowIcon);
    //
    //// media Player
    //disconnect(m_spUi->pResourceDisplay, &CResourceDisplayWidget::SignalLoadFinished,
    //        m_spUi->pResourceDisplay, &CResourceDisplayWidget::SlotPlayPause);
    //disconnect(m_spUi->pSoundEmmiter, &CResourceDisplayWidget::SignalLoadFinished,
    //        m_spUi->pSoundEmmiter, &CResourceDisplayWidget::SlotPlayPause);
    //disconnect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalPauseVideo,
    //        this, &CSceneMainScreen::SlotPauseVideo);
    //disconnect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalPauseSound,
    //        this, &CSceneMainScreen::SlotPauseSound);
    //disconnect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalPlayMedia,
    //        this, &CSceneMainScreen::SlotPlayMedia);
    //disconnect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalShowMedia,
    //        this, &CSceneMainScreen::SlotShowMedia);
    //disconnect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalStopVideo,
    //        this, &CSceneMainScreen::SlotStopVideo);
    //disconnect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalStopSound,
    //        this, &CSceneMainScreen::SlotStopSound);
    //
    //// TextBox
    //disconnect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalClearText,
    //        m_spUi->pTextBoxDisplay, &CTextBoxWidget::SlotClearText);
    //disconnect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalShowButtonPrompts,
    //        m_spUi->pTextBoxDisplay, &CTextBoxWidget::SlotShowButtonPrompts);
    //disconnect(m_spUi->pTextBoxDisplay, &CTextBoxWidget::SignalShowButtonReturnValue,
    //        spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalShowButtonReturnValue);
    //disconnect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalShowInput,
    //        m_spUi->pTextBoxDisplay, &CTextBoxWidget::SlotShowInput);
    //disconnect(m_spUi->pTextBoxDisplay, &CTextBoxWidget::SignalShowInputReturnValue,
    //        spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalShowInputReturnValue);
    //disconnect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalShowText,
    //        m_spUi->pTextBoxDisplay, &CTextBoxWidget::SlotShowText);
    //disconnect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalTextBackgroundColorsChanged,
    //        m_spUi->pTextBoxDisplay, &CTextBoxWidget::SlotTextBackgroundColorsChanged);
    //disconnect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalTextColorsChanged,
    //        m_spUi->pTextBoxDisplay, &CTextBoxWidget::SlotTextColorsChanged);
    //
    //// timer
    //disconnect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalHideTimer,
    //        m_spUi->pTimerDisplay, &CTimerDisplayWidget::SlotHideTimer);
    //disconnect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalSetTime,
    //        m_spUi->pTimerDisplay, &CTimerDisplayWidget::SlotSetTime);
    //disconnect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalSetTimeVisible,
    //        m_spUi->pTimerDisplay, &CTimerDisplayWidget::SlotSetTimeVisible);
    //disconnect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalHideTimer,
    //        m_spUi->pTimerDisplay, &CTimerDisplayWidget::SlotHideTimer);
    //disconnect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalShowTimer,
    //        m_spUi->pTimerDisplay, &CTimerDisplayWidget::SlotShowTimer);
    //disconnect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalStartTimer,
    //        m_spUi->pTimerDisplay, &CTimerDisplayWidget::SlotStartTimer);
    //disconnect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalStopTimer,
    //        m_spUi->pTimerDisplay, &CTimerDisplayWidget::SlotStopTimer);
    //disconnect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalWaitForTimer,
    //        m_spUi->pTimerDisplay, &CTimerDisplayWidget::SlotWaitForTimer);
    //disconnect(m_spUi->pTimerDisplay, &CTimerDisplayWidget::SignalTimerFinished,
    //        spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalTimerFinished);
    //
    //// Thread
    //disconnect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalSkippableWait,
    //        m_spUi->pInfoDisplay, &CInformationWidget::SlotShowSkipIcon);
    //disconnect(m_spUi->pInfoDisplay, &CInformationWidget::SignalWaitSkipped,
    //        spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalWaitSkipped);
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

  //m_spUi->pTextBoxDisplay->SlotClearText();

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
      //m_spUi->pTextBoxDisplay->SetSceneSelection(true);
      //m_spUi->pTextBoxDisplay->SlotShowButtonPrompts(sScenes);
    }
  }
  else
  {
    qInfo() << tr("No more scenes to load.");
    SlotQuit();
  }
}
