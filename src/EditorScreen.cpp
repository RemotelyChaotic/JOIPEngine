#include "EditorScreen.h"
#include "ui_EditorScreen.h"

CEditorScreen::CEditorScreen(const std::shared_ptr<CWindowContext>& spWindowContext,
                             QWidget* pParent) :
  QWidget(pParent),
  IAppStateScreen(spWindowContext),
  m_spUi(std::make_unique<Ui::CEditorScreen>())
{
  m_spUi->setupUi(this);
  Initialize();
}

CEditorScreen::~CEditorScreen()
{

}

//----------------------------------------------------------------------------------------
//
void CEditorScreen::Initialize()
{
  m_bInitialized = false;


  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CEditorScreen::Load()
{

}

//----------------------------------------------------------------------------------------
//
void CEditorScreen::Unload()
{

}
