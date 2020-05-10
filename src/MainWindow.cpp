#include "MainWindow.h"
#include "Application.h"
#include "CreditsScreen.h"
#include "Enums.h"
#include "EditorScreen.h"
#include "MainScreen.h"
#include "SceneScreen.h"
#include "SceneScreen.h"
#include "Settings.h"
#include "SettingsScreen.h"
#include "WindowContext.h"
#include "Widgets/BackgroundWidget.h"
#include "Widgets/HelpOverlay.h"
#include "ui_MainWindow.h"
#include <QDesktopWidget>
#include <QStyle>

CMainWindow::CMainWindow(QWidget* pParent) :
  QMainWindow(pParent),
  m_spUi(std::make_unique<Ui::CMainWindow>()),
  m_spHelpButtonOverlay(std::make_unique<CHelpButtonOverlay>(this)),
  m_spHelpOverlay(nullptr),
  m_spWindowContext(std::make_shared<CWindowContext>()),
  m_pBackground(new CBackgroundWidget(this)),
  m_bInitialized(false)
{
  m_spHelpOverlay.reset(new CHelpOverlay(m_spHelpButtonOverlay.get(), this));
  m_spUi->setupUi(this);
  setWindowFlags(Qt::FramelessWindowHint);

  m_pBackground->setFixedSize(size());
  m_pBackground->lower();
}

CMainWindow::~CMainWindow()
{
  m_spHelpOverlay.reset();
  m_spHelpButtonOverlay.reset();
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

  ConnectSlots();

  SlotResolutionChanged();
  SlotFullscreenChanged();

  m_spUi->pApplicationStackWidget->setCurrentIndex(EAppState::eMainScreen);
  IAppStateScreen* pScreen =
    dynamic_cast<IAppStateScreen*>(m_spUi->pApplicationStackWidget->currentWidget());
  if (nullptr != pScreen)
  {
    pScreen->Load();
  }

  m_spHelpButtonOverlay->Initialize();
  connect(m_spHelpButtonOverlay.get(), &CHelpButtonOverlay::SignalButtonClicked,
          this, &CMainWindow::SlotHelpButtonClicked);
  m_spHelpButtonOverlay->Show();

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CMainWindow::SlotChangeAppState(EAppState newState)
{
  IAppStateScreen* pScreen =
    dynamic_cast<IAppStateScreen*>(m_spUi->pApplicationStackWidget->currentWidget());
  if (nullptr != pScreen)
  {
    pScreen->Unload();
  }

  m_spUi->pApplicationStackWidget->setCurrentIndex(newState);

  m_spHelpButtonOverlay->Show();

  pScreen =
      dynamic_cast<IAppStateScreen*>(m_spUi->pApplicationStackWidget->currentWidget());
  if (nullptr != pScreen)
  {
    pScreen->Load();
  }
}

//----------------------------------------------------------------------------------------
//
void CMainWindow::SlotFullscreenChanged()
{
  bool bFullscreen = m_spSettings->Fullscreen();
  if (bFullscreen)
  {
    setWindowState(windowState() | Qt::WindowFullScreen);
  }
  else
  {
    setWindowState(windowState() & ~Qt::WindowFullScreen);
  }
}

//----------------------------------------------------------------------------------------
//
void CMainWindow::SlotResolutionChanged()
{
  QSize newResolution = m_spSettings->Resolution();
  setGeometry(
      QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter,
          newResolution, QApplication::desktop()->availableGeometry()));
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
void CMainWindow::resizeEvent(QResizeEvent* pEvent)
{
  m_pBackground->setFixedSize(pEvent->size());
  m_pBackground->setGeometry(0, 0, pEvent->size().width(), pEvent->size().height());
}

//----------------------------------------------------------------------------------------
//
void CMainWindow::SlotHelpButtonClicked()
{
  m_spHelpOverlay->Show(mapToGlobal(m_spHelpButtonOverlay->geometry().center()),
                        m_spUi->pApplicationStackWidget->currentWidget());
}

//----------------------------------------------------------------------------------------
//
void CMainWindow::ConnectSlots()
{
  connect(m_spWindowContext.get(), &CWindowContext::SignalChangeAppState,
          this, &CMainWindow::SlotChangeAppState, Qt::DirectConnection);

  connect(m_spWindowContext.get(), &CWindowContext::SignalSetHelpButtonVisible,
          this, &CMainWindow::SlotSetHelpButtonVisible, Qt::DirectConnection);

  connect(m_spSettings.get(), &CSettings::fullscreenChanged,
          this, &CMainWindow::SlotFullscreenChanged, Qt::QueuedConnection);

  connect(m_spSettings.get(), &CSettings::resolutionChanged,
          this, &CMainWindow::SlotResolutionChanged, Qt::QueuedConnection);
}
