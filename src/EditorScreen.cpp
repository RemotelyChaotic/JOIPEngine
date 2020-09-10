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
  connect(m_spUi->pChoiceScreen, &CEditorChoiceScreen::SignalUnloadFinished,
          this, &CEditorScreen::UnloadFinished, Qt::DirectConnection);

  connect(m_spUi->pEditorScreen, &CEditorMainScreen::SignalExitClicked,
          this, &CEditorScreen::SlotExitClicked, Qt::DirectConnection);
  connect(m_spUi->pEditorScreen, &CEditorMainScreen::SignalUnloadFinished,
          this, &CEditorScreen::UnloadFinished, Qt::DirectConnection);

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
void CEditorScreen::SlotNewClicked(const QString& sNewProjectName, bool bTutorial)
{
  WIDGET_INITIALIZED_GUARD

  m_spUi->pChoiceScreen->Unload();

  emit m_spWindowContext->SignalSetHelpButtonVisible(false);

  m_spUi->pEditorScreen->InitNewProject(sNewProjectName, bTutorial);
  m_spUi->pStackedWidget->setCurrentIndex(c_iPageIndexEditor);
}

//----------------------------------------------------------------------------------------
//
void CEditorScreen::SlotOpenClicked(qint32 iId)
{
  WIDGET_INITIALIZED_GUARD

  m_spUi->pChoiceScreen->Unload();

  emit m_spWindowContext->SignalSetHelpButtonVisible(false);

  m_spUi->pEditorScreen->LoadProject(iId);
  m_spUi->pStackedWidget->setCurrentIndex(c_iPageIndexEditor);
}

//----------------------------------------------------------------------------------------
//
void CEditorScreen::SlotCancelClicked()
{
  WIDGET_INITIALIZED_GUARD
  m_spUi->pStackedWidget->setCurrentIndex(c_iPageIndexChoice);
  emit m_spWindowContext->SignalChangeAppState(EAppState::eMainScreen);
}

//----------------------------------------------------------------------------------------
//
void CEditorScreen::SlotExitClicked()
{
  WIDGET_INITIALIZED_GUARD
  m_spUi->pStackedWidget->setCurrentIndex(c_iPageIndexChoice);
  emit m_spWindowContext->SignalChangeAppState(EAppState::eMainScreen);
}
