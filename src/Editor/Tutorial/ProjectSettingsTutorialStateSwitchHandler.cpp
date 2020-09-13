#include "ProjectSettingsTutorialStateSwitchHandler.h"
#include "Editor/EditorProjectSettingsWidget.h"

CProjectSettingsTutorialStateSwitchHandler::
CProjectSettingsTutorialStateSwitchHandler(QPointer<CEditorProjectSettingsWidget> pParentWidget,
                                           const std::shared_ptr<Ui::CEditorProjectSettingsWidget>& spUi) :
  ITutorialStateSwitchHandler(),
  m_pParentWidget(pParentWidget),
  m_spUi(spUi)
{

}

CProjectSettingsTutorialStateSwitchHandler::~CProjectSettingsTutorialStateSwitchHandler()
{

}

//----------------------------------------------------------------------------------------
//
void CProjectSettingsTutorialStateSwitchHandler::OnResetStates()
{

}

//----------------------------------------------------------------------------------------
//
void CProjectSettingsTutorialStateSwitchHandler::OnStateSwitch(ETutorialState newState,
                                                               ETutorialState oldstate)
{

}
