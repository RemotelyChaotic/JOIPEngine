#ifndef CMAINSCREENTUTORIALSTATESWITCHER_H
#define CMAINSCREENTUTORIALSTATESWITCHER_H

#include "ITutorialStateSwitchHandler.h"
#include <QPointer>
#include <memory>

class CEditorMainScreen;
class CEditorTutorialOverlay;
class CJsonInstructionSetParser;
class CJsonInstructionSetRunner;
namespace Ui {
  class CEditorMainScreen;
}

class CMainScreenTutorialStateSwitchHandler : public ITutorialStateSwitchHandler
{
public:
  CMainScreenTutorialStateSwitchHandler(QPointer<CEditorMainScreen> pParentWidget,
                                        const std::shared_ptr<Ui::CEditorMainScreen>& spUi,
                                        QPointer<CEditorTutorialOverlay> pTutorialOverlay);
  ~CMainScreenTutorialStateSwitchHandler() override;

  void OnResetStates() override;
  void OnStateSwitch(ETutorialState newState, ETutorialState oldState) override;

private:
  std::unique_ptr<CJsonInstructionSetParser> m_spTutorialParser;
  std::shared_ptr<Ui::CEditorMainScreen>     m_spUi;
  std::shared_ptr<CJsonInstructionSetRunner> m_spTutorialRunner;
  QPointer<CEditorMainScreen>                m_ParentWidget;
  QPointer<CEditorTutorialOverlay>           m_pTutorialOverlay;
};

#endif // CMAINSCREENTUTORIALSTATESWITCHER_H
