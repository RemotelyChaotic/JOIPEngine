#include "EditorScreen.h"
#include "ui_EditorScreen.h"

namespace
{
  const qint32 c_iPageIndexChoice = 0;
  const qint32 c_iPageIndexEditor = 1;
}

//----------------------------------------------------------------------------------------
//
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

  connect(m_spUi->pChoiceScreen, &CEditorChoiceScreen::SignalNewClicked,
          this, &CEditorScreen::SlotNewClicked, Qt::DirectConnection);
  connect(m_spUi->pChoiceScreen, &CEditorChoiceScreen::SignalOpenClicked,
          this, &CEditorScreen::SlotOpenClicked, Qt::DirectConnection);
  connect(m_spUi->pChoiceScreen, &CEditorChoiceScreen::SignalCancelClicked,
          this, &CEditorScreen::SlotCancelClicked, Qt::DirectConnection);

  m_spUi->pStackedWidget->setCurrentIndex(c_iPageIndexChoice);

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CEditorScreen::Load()
{
  m_spUi->pChoiceScreen->Load();
}

//----------------------------------------------------------------------------------------
//
void CEditorScreen::Unload()
{
  m_spUi->pChoiceScreen->Unload();
}

//----------------------------------------------------------------------------------------
//
void CEditorScreen::SlotNewClicked(const QString& sNewProjectName)
{
  SCREEN_INITIALIZED_GUARD

  m_spUi->pEditorScreen->UnloadProject();
  m_spUi->pEditorScreen->InitNewProject(sNewProjectName);
  m_spUi->pStackedWidget->setCurrentIndex(c_iPageIndexEditor);
}

//----------------------------------------------------------------------------------------
//
void CEditorScreen::SlotOpenClicked(qint32 iId)
{
  SCREEN_INITIALIZED_GUARD

  m_spUi->pEditorScreen->UnloadProject();
  m_spUi->pEditorScreen->LoadProject(iId);
  m_spUi->pStackedWidget->setCurrentIndex(c_iPageIndexEditor);
}

//----------------------------------------------------------------------------------------
//
void CEditorScreen::SlotCancelClicked()
{
  SCREEN_INITIALIZED_GUARD
  emit m_spWindowContext->SignalChangeAppState(EAppState::eMainScreen);
}
