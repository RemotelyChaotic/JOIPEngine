#include "MainScreenTutorialStateSwitchHandler.h"
#include "Editor/EditorMainScreen.h"

CMainScreenTutorialStateSwitchHandler::CMainScreenTutorialStateSwitchHandler(
    QPointer<CEditorMainScreen> pParentWidget,
    const std::shared_ptr<Ui::CEditorMainScreen>& spUi) :
  ITutorialStateSwitchHandler(),
  m_spUi(spUi),
  m_ParentWidget(pParentWidget)
{

}

CMainScreenTutorialStateSwitchHandler::~CMainScreenTutorialStateSwitchHandler()
{

}

//----------------------------------------------------------------------------------------
//
void CMainScreenTutorialStateSwitchHandler::OnResetStates()
{

}

//----------------------------------------------------------------------------------------
//
void CMainScreenTutorialStateSwitchHandler::OnStateSwitch(ETutorialState newState,
                                                          ETutorialState oldstate)
{

}
