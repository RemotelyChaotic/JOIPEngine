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
  m_spUi->pProjectCardSelectionWidget->LoadProjects();
}

//----------------------------------------------------------------------------------------
//
void CSceneSelectionScreen::Unload()
{
  m_spUi->pProjectCardSelectionWidget->UnloadProjects();
}

//----------------------------------------------------------------------------------------
//
void CSceneSelectionScreen::on_pOpenExistingProjectButton_clicked()
{
  SCREEN_INITIALIZED_GUARD

  // TODO: implement properly

  emit m_spWindowContext->SignalChangeAppState(EAppState::eSceneScreen);
}

//----------------------------------------------------------------------------------------
//
void CSceneSelectionScreen::on_pCancelButton_clicked()
{
  SCREEN_INITIALIZED_GUARD

  emit m_spWindowContext->SignalChangeAppState(EAppState::eMainScreen);
}
