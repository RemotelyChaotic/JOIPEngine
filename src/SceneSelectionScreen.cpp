#include "SceneSelectionScreen.h"
#include "WindowContext.h"
#include "ui_SceneSelectionScreen.h"

CSceneSelectionScreen::CSceneSelectionScreen(const std::shared_ptr<CWindowContext>& spWindowContext,
                                             QWidget* pParent) :
  QWidget(pParent),
  IAppStateScreen(spWindowContext),
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

//----------------------------------------------------------------------------------------
//
void CSceneSelectionScreen::Load()
{

}

//----------------------------------------------------------------------------------------
//
void CSceneSelectionScreen::Unload()
{

}
