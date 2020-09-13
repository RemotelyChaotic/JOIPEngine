#include "MainScreenTutorialStateSwitchHandler.h"
#include "EditorTutorialOverlay.h"
#include "Editor/EditorMainScreen.h"
#include <QTimer>

CMainScreenTutorialStateSwitchHandler::CMainScreenTutorialStateSwitchHandler(
    QPointer<CEditorMainScreen> pParentWidget,
    const std::shared_ptr<Ui::CEditorMainScreen>& spUi,
    QPointer<CEditorTutorialOverlay> pTutorialOverlay) :
  ITutorialStateSwitchHandler(),
  m_spUi(spUi),
  m_ParentWidget(pParentWidget),
  m_pTutorialOverlay(pTutorialOverlay)
{

}

CMainScreenTutorialStateSwitchHandler::~CMainScreenTutorialStateSwitchHandler()
{

}

//----------------------------------------------------------------------------------------
//
void CMainScreenTutorialStateSwitchHandler::OnResetStates()
{
  m_pTutorialOverlay->Hide();
}

//----------------------------------------------------------------------------------------
//
void CMainScreenTutorialStateSwitchHandler::OnStateSwitch(ETutorialState newState,
                                                          ETutorialState oldState)
{
  if (oldState._to_integral() == ETutorialState::eUnstarted)
  {
    // deleay showing a bit, so the user can see the UI pop up
    QTimer::singleShot(500, m_pTutorialOverlay, SLOT(Show()));
  }

  switch (newState)
  {
    case ETutorialState::eBeginTutorial:
    {

    } break;
  default: break;
  }
}
