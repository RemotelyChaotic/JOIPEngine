#include "SceneNodeWidgetTutorialStateSwitchHandler.h"
#include "Editor/EditorSceneNodeWidget.h"

CSceneNodeWidgetTutorialStateSwitchHandler::
CSceneNodeWidgetTutorialStateSwitchHandler(QPointer<CEditorSceneNodeWidget> pParentWidget,
                                           const std::shared_ptr<Ui::CEditorSceneNodeWidget>& spUi) :
  ITutorialStateSwitchHandler(),
  m_pParentWidget(pParentWidget),
  m_spUi(spUi)
{

}

CSceneNodeWidgetTutorialStateSwitchHandler::~CSceneNodeWidgetTutorialStateSwitchHandler()
{

}

//----------------------------------------------------------------------------------------
//
void CSceneNodeWidgetTutorialStateSwitchHandler::OnResetStates()
{

}

//----------------------------------------------------------------------------------------
//
void CSceneNodeWidgetTutorialStateSwitchHandler::OnStateSwitch(ETutorialState newState,
                                                               ETutorialState oldstate)
{

}
