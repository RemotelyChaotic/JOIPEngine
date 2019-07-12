#include "SettingsScreen.h"
#include "ui_SettingsScreen.h"

CSettingsScreen::CSettingsScreen(QWidget *parent) :
  QWidget(parent),
  m_spUi(std::make_unique<Ui::CSettingsScreen>())
{
  m_spUi->setupUi(this);
  Initialize();
}

CSettingsScreen::~CSettingsScreen()
{

}

//----------------------------------------------------------------------------------------
//
void CSettingsScreen::Initialize()
{
  m_bInitialized = false;


  m_bInitialized = true;
}
