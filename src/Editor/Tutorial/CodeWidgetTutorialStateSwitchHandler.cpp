#include "CodeWidgetTutorialStateSwitchHandler.h"
#include "Editor/EditorWidgets/EditorCodeWidget.h"

CCodeWidgetTutorialStateSwitchHandler::CCodeWidgetTutorialStateSwitchHandler(QPointer<CEditorCodeWidget> pParentWidget,
                                                                             const std::shared_ptr<Ui::CEditorCodeWidget>& spUI) :
  ITutorialStateSwitchHandler(),
  m_pParentWidget(pParentWidget),
  m_spUi(spUI)
{
}

CCodeWidgetTutorialStateSwitchHandler::~CCodeWidgetTutorialStateSwitchHandler()
{

}

//----------------------------------------------------------------------------------------
//
void CCodeWidgetTutorialStateSwitchHandler::OnResetStates()
{

}

//----------------------------------------------------------------------------------------
//
void CCodeWidgetTutorialStateSwitchHandler::OnStateSwitch(ETutorialState newState,
                                                          ETutorialState oldstate)
{
  Q_UNUSED(newState)
  Q_UNUSED(oldstate)
}
