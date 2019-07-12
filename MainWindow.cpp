#include "MainWindow.h"
#include "MainScreen.h"
#include "SceneScreen.h"
#include "SceneSelectionScreen.h"
#include "SettingsScreen.h"
#include "ui_MainWindow.h"

CMainWindow::CMainWindow(QWidget *parent) :
  QMainWindow(parent),
  m_spUi(std::make_unique<Ui::CMainWindow>()),
  m_bInitialized(false)
{
  m_spUi->setupUi(this);
  Initialize();
}

CMainWindow::~CMainWindow()
{
}

//----------------------------------------------------------------------------------------
//
void CMainWindow::Initialize()
{
  m_bInitialized = false;

  m_spUi->pMainToolBar->setVisible(false);
  m_spUi->pMenuBar->setVisible(false);
  m_spUi->pStatusBar->setVisible(false);

  m_spUi->pApplicationStackWidget->addWidget(new CMainScreen(m_spUi->pApplicationStackWidget));
  m_spUi->pApplicationStackWidget->addWidget(new CSceneSelectionScreen(m_spUi->pApplicationStackWidget));
  m_spUi->pApplicationStackWidget->addWidget(new CSettingsScreen(m_spUi->pApplicationStackWidget));
  m_spUi->pApplicationStackWidget->addWidget(new CSceneScreen(m_spUi->pApplicationStackWidget));

  m_bInitialized = true;
}
