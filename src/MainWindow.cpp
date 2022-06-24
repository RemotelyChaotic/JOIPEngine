#include "MainWindow.h"
#include "Application.h"
#include "CreditsScreen.h"
#include "DownloadScreen.h"
#include "Enums.h"
#include "EditorScreen.h"
#include "MainScreen.h"
#include "SceneScreen.h"
#include "SceneScreen.h"
#include "Settings.h"
#include "SettingsScreen.h"
#include "WindowContext.h"
#include "Systems/BackActionHandler.h"
#include "Systems/ProjectDownloader.h"
#include "Widgets/AgeCheckOverlay.h"
#include "Widgets/BackgroundWidget.h"
#include "Widgets/DownloadButtonOverlay.h"
#include "Widgets/HelpOverlay.h"
#include "ui_MainWindow.h"

#include <QDesktopWidget>
#include <QDir>
#include <QMessageBox>

CMainWindow::CMainWindow(QWidget* pParent) :
  CMainWindowBase(pParent),
  m_spUi(std::make_unique<Ui::CMainWindow>()),
  m_spAgeCheckOverlay(std::make_unique<CAgeCheckOverlay>(this)),
  m_spHelpButtonOverlay(std::make_unique<CHelpButtonOverlay>(this)),
  m_spHelpOverlay(nullptr),
  m_spDownloadButtonOverlay(std::make_unique<CDownloadButtonOverlay>(this)),
  m_spWindowContext(std::make_shared<CWindowContext>()),
  m_pBackground(new CBackgroundWidget(this)),
  m_bInitialized(false),
  m_nextState(EAppState::eMainScreen)
{
  m_spHelpOverlay.reset(new CHelpOverlay(m_spHelpButtonOverlay.get(), this));
  m_spUi->setupUi(this);

  m_pBackground->setFixedSize(size());
  m_pBackground->lower();
}

CMainWindow::~CMainWindow()
{
  m_spHelpOverlay.reset();
  m_spHelpButtonOverlay.reset();
  m_spDownloadButtonOverlay.reset();
}

//----------------------------------------------------------------------------------------
//
void CMainWindow::ConnectSlots()
{
  connect(m_spWindowContext.get(), &CWindowContext::SignalChangeAppState,
          this, &CMainWindow::SlotChangeAppState, Qt::DirectConnection);

  connect(m_spWindowContext.get(), &CWindowContext::SignalSetDownloadButtonVisible,
          this, &CMainWindow::SlotSetDownloadButtonVisible, Qt::DirectConnection);

  connect(m_spWindowContext.get(), &CWindowContext::SignalSetHelpButtonVisible,
          this, &CMainWindow::SlotSetHelpButtonVisible, Qt::DirectConnection);

  ConnectSlotsImpl();
}

//----------------------------------------------------------------------------------------
//
void CMainWindow::Initialize()
{
  m_bInitialized = false;

  m_spSettings = CApplication::Instance()->Settings();

  m_spUi->pMainToolBar->setVisible(false);
  m_spUi->pMenuBar->setVisible(false);
  m_spUi->pStatusBar->setVisible(false);

  m_spHelpButtonOverlay->Initialize();
  connect(m_spHelpButtonOverlay.get(), &CHelpButtonOverlay::SignalButtonClicked,
          this, &CMainWindow::SlotHelpButtonClicked);
  m_spHelpButtonOverlay->Show();

  m_spDownloadButtonOverlay->Initialize();
  connect(m_spDownloadButtonOverlay.get(), &CHelpButtonOverlay::SignalButtonClicked,
          this, &CMainWindow::SlotDownloadButtonClicked);
  m_spDownloadButtonOverlay->Show();

  m_spAgeCheckOverlay->Show();

  // add all screens
  m_spUi->pApplicationStackWidget->insertWidget(EAppState::eMainScreen,
   new CMainScreen(m_spWindowContext, m_spUi->pApplicationStackWidget));
  m_spUi->pApplicationStackWidget->insertWidget(EAppState::eSceneScreen,
   new CSceneScreen(m_spWindowContext, m_spUi->pApplicationStackWidget));
  m_spUi->pApplicationStackWidget->insertWidget(EAppState::eSettingsScreen,
   new CSettingsScreen(m_spWindowContext, m_spUi->pApplicationStackWidget));
  m_spUi->pApplicationStackWidget->insertWidget(EAppState::eEditorScreen,
   new CEditorScreen(m_spWindowContext, m_spUi->pApplicationStackWidget));
  m_spUi->pApplicationStackWidget->insertWidget(EAppState::eCreditsScreen,
   new CCreditsScreen(m_spWindowContext, m_spUi->pApplicationStackWidget));
  m_spUi->pApplicationStackWidget->insertWidget(EAppState::eDownloadScreen,
   new CDownloadScreen(m_spWindowContext, m_spUi->pApplicationStackWidget));

  ConnectSlots();

  SlotResolutionChanged();
  SlotWindowModeChanged();

  m_spUi->pApplicationStackWidget->setCurrentIndex(EAppState::eMainScreen);
  IAppStateScreen* pScreen =
    dynamic_cast<IAppStateScreen*>(m_spUi->pApplicationStackWidget->currentWidget());
  if (nullptr != pScreen)
  {
    pScreen->Load();
  }

  if (m_spSettings->HasOldSettingsVersion())
  {
    bool bOk = QMetaObject::invokeMethod(this, "OldSettingsDetected", Qt::QueuedConnection);
    assert(bOk); Q_UNUSED(bOk)
  }

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CMainWindow::SlotChangeAppState(EAppState newState)
{
  m_nextState = newState;

  IAppStateScreen* pScreen =
    dynamic_cast<IAppStateScreen*>(m_spUi->pApplicationStackWidget->currentWidget());
  if (nullptr != pScreen)
  {
    bool bOk = connect(dynamic_cast<QWidget*>(pScreen), SIGNAL(UnloadFinished()),
                       this, SLOT(SlotCurrentAppStateUnloadFinished()));
    assert(bOk);
    Q_UNUSED(bOk);
    pScreen->Unload();
  }
}

//----------------------------------------------------------------------------------------
//
void CMainWindow::SlotChangeAppStateToMain()
{
  SlotChangeAppState(EAppState::eMainScreen);
}

//----------------------------------------------------------------------------------------
//
void CMainWindow::SlotCurrentAppStateUnloadFinished()
{
  IAppStateScreen* pScreen =
    dynamic_cast<IAppStateScreen*>(m_spUi->pApplicationStackWidget->currentWidget());
  if (nullptr != pScreen)
  {
    bool bOk = disconnect(dynamic_cast<QWidget*>(pScreen), SIGNAL(UnloadFinished()),
                       this, SLOT(SlotCurrentAppStateUnloadFinished()));
    assert(bOk);
    Q_UNUSED(bOk);
  }

  m_spUi->pApplicationStackWidget->setCurrentIndex(m_nextState);

  m_spHelpButtonOverlay->Show();
  m_spDownloadButtonOverlay->Show();

  pScreen =
      dynamic_cast<IAppStateScreen*>(m_spUi->pApplicationStackWidget->currentWidget());
  if (nullptr != pScreen)
  {
    pScreen->Load();
  }

  if (auto spBackActionHandler = CApplication::Instance()->System<CBackActionHandler>().lock())
  {
    if (EAppState::eMainScreen != m_nextState._to_integral())
    {
      spBackActionHandler->RegisterSlotToCall(this, "SlotChangeAppStateToMain");
    }
    else
    {
      spBackActionHandler->ClearSlotToCall();
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CMainWindow::SlotDownloadButtonClicked()
{
  if (m_spUi->pApplicationStackWidget->currentIndex() != EAppState::eDownloadScreen)
  {
    SlotChangeAppState(EAppState::eDownloadScreen);
  }
  else
  {
    SlotChangeAppState(EAppState::eMainScreen);
  }
}

//----------------------------------------------------------------------------------------
//
void CMainWindow::SlotSetDownloadButtonVisible(bool bVisible)
{
  if (bVisible)
  {
    m_spDownloadButtonOverlay->Show();
  }
  else
  {
    m_spDownloadButtonOverlay->Hide();
  }
}

//----------------------------------------------------------------------------------------
//
void CMainWindow::SlotSetHelpButtonVisible(bool bVisible)
{
  if (bVisible)
  {
    m_spHelpButtonOverlay->Show();
  }
  else
  {
    m_spHelpButtonOverlay->Hide();
  }
}

//----------------------------------------------------------------------------------------
//
void CMainWindow::closeEvent(QCloseEvent* pEvent)
{
  if (auto spPrjDownloader = CApplication::Instance()->System<CProjectDownloader>().lock())
  {
    if (spPrjDownloader->HasRunningJobs())
    {
      QPointer<CMainWindow> pThis(this);
      QMessageBox::StandardButton resBtn =
          QMessageBox::question(this, QString("Downloads running."),
                                tr("Downloads are still running and will be canceled.\n"
                                   "If you quit, you will need to delete the left-over folders manually.\n"
                                   "Are you sure you want to quit?\n"),
                                QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
                                QMessageBox::No);
      if (nullptr == pThis) { return; } // abort, abort window was destroyed
      if (resBtn != QMessageBox::Yes)
      {
        spPrjDownloader->ClearQueue();
        spPrjDownloader->StopRunningJobs();
        spPrjDownloader->WaitForFinished();
        pEvent->accept();
        return;
      }
      else
      {
        pEvent->ignore();
        return;
      }
    }
  }
  pEvent->accept();
}

//----------------------------------------------------------------------------------------
//
void CMainWindow::resizeEvent(QResizeEvent* pEvent)
{
  Q_UNUSED(pEvent)
  m_pBackground->setFixedSize(size());
  m_pBackground->setGeometry(0, 0, width(), height());
}

//----------------------------------------------------------------------------------------
//
void CMainWindow::SlotHelpButtonClicked()
{
  m_spHelpOverlay->Show(mapFromGlobal(
                          m_spHelpButtonOverlay->parentWidget()->mapToGlobal(
                              m_spHelpButtonOverlay->geometry().center())),
                        m_spUi->pApplicationStackWidget->currentWidget());
}
