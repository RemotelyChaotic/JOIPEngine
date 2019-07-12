#include "SceneSelectionScreen.h"
#include "ui_SceneSelectionScreen.h"

CSceneSelectionScreen::CSceneSelectionScreen(QWidget *parent) :
  QWidget(parent),
  m_spUi(std::make_unique<Ui::CSceneSelectionScreen>())
{
  m_spUi->setupUi(this);
  Initialize();
}

CSceneSelectionScreen::~CSceneSelectionScreen()
{
}

//----------------------------------------------------------------------------------------
//
void CSceneSelectionScreen::Initialize()
{
  m_bInitialized = false;


  m_bInitialized = true;
}
