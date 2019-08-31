#include "MainScreen.h"
#include "WindowContext.h"
#include "ui_MainScreen.h"
#include <QApplication>

CMainScreen::CMainScreen(const std::shared_ptr<CWindowContext>& spWindowContext,
                         QWidget* pParent) :
  QWidget(pParent),
  IAppStateScreen(spWindowContext),
  m_spUi(std::make_unique<Ui::CMainScreen>())
{
  m_spUi->setupUi(this);
  Initialize();
}

CMainScreen::~CMainScreen()
{
}

//----------------------------------------------------------------------------------------
//
void CMainScreen::Initialize()
{
  m_bInitialized = false;


  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CMainScreen::Load()
{

}

//----------------------------------------------------------------------------------------
//
void CMainScreen::Unload()
{

}

//----------------------------------------------------------------------------------------
//
void CMainScreen::on_pSceneSelectButton_clicked()
{
  WIDGET_INITIALIZED_GUARD

  emit m_spWindowContext->SignalChangeAppState(EAppState::eSceneScreen);
}

//----------------------------------------------------------------------------------------
//
void CMainScreen::on_pEdiorButton_clicked()
{
  WIDGET_INITIALIZED_GUARD

  emit m_spWindowContext->SignalChangeAppState(EAppState::eEditorScreen);
}

//----------------------------------------------------------------------------------------
//
void CMainScreen::on_pSettingsButton_clicked()
{
  WIDGET_INITIALIZED_GUARD

  emit m_spWindowContext->SignalChangeAppState(EAppState::eSettingsScreen);
}

//----------------------------------------------------------------------------------------
//
void CMainScreen::on_pQuitButton_clicked()
{
  WIDGET_INITIALIZED_GUARD

  emit QApplication::instance()->quit();
}
