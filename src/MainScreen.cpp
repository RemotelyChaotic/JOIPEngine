#include "MainScreen.h"
#include "ui_MainScreen.h"

CMainScreen::CMainScreen(QWidget *parent) :
  QWidget(parent),
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
