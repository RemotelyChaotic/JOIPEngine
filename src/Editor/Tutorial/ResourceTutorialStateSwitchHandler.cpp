#include "ResourceTutorialStateSwitchHandler.h"
#include "Editor/EditorResourceWidget.h"

CResourceTutorialStateSwitchHandler::
CResourceTutorialStateSwitchHandler(QPointer<CEditorResourceWidget> pParentWidget,
                                    const std::shared_ptr<Ui::CEditorResourceWidget>& spUi) :
  ITutorialStateSwitchHandler(),
  m_pParentWidget(pParentWidget),
  m_spUi(spUi)
{
}

CResourceTutorialStateSwitchHandler::~CResourceTutorialStateSwitchHandler()
{

}

//----------------------------------------------------------------------------------------
//
void CResourceTutorialStateSwitchHandler::OnResetStates()
{

}

//----------------------------------------------------------------------------------------
//
void CResourceTutorialStateSwitchHandler::OnStateSwitch(ETutorialState newState,
                                                        ETutorialState oldstate)
{

}
