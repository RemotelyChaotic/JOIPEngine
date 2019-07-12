#include "SceneScreen.h"
#include "ui_SceneScreen.h"

CSceneScreen::CSceneScreen(QWidget *parent) :
  QWidget(parent),
  m_spUi(std::make_unique<Ui::CSceneScreen>())
{
  m_spUi->setupUi(this);
  Initialize();;
}

CSceneScreen::~CSceneScreen()
{
}

//----------------------------------------------------------------------------------------
//
void CSceneScreen::Initialize()
{
  m_bInitialized = false;


  m_bInitialized = true;
}
