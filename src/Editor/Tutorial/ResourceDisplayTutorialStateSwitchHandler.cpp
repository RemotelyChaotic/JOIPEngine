#include "ResourceDisplayTutorialStateSwitchHandler.h"
#include "Editor/EditorResourceDisplayWidget.h"

CResourceDisplayTutorialStateSwitchHandler::
CResourceDisplayTutorialStateSwitchHandler(QPointer<CEditorResourceDisplayWidget> pParentWidget,
                                           const std::shared_ptr<Ui::CEditorResourceDisplayWidget>& spUi) :
  ITutorialStateSwitchHandler(),
  m_pParentWidget(pParentWidget),
  m_spUi(spUi)
{

}

CResourceDisplayTutorialStateSwitchHandler::~CResourceDisplayTutorialStateSwitchHandler()
{

}

//----------------------------------------------------------------------------------------
//
void CResourceDisplayTutorialStateSwitchHandler::OnResetStates()
{

}

//----------------------------------------------------------------------------------------
//
void CResourceDisplayTutorialStateSwitchHandler::OnStateSwitch(ETutorialState newState,
                                                               ETutorialState oldstate)
{

}
