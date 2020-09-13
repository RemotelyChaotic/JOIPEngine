#ifndef CMAINSCREENTUTORIALSTATESWITCHER_H
#define CMAINSCREENTUTORIALSTATESWITCHER_H

#include "ITutorialStateSwitchHandler.h"
#include <QPointer>
#include <memory>

class CEditorMainScreen;
class CEditorTutorialOverlay;
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
  std::shared_ptr<Ui::CEditorMainScreen>   m_spUi;
  QPointer<CEditorMainScreen>              m_ParentWidget;
  QPointer<CEditorTutorialOverlay>         m_pTutorialOverlay;
};

#endif // CMAINSCREENTUTORIALSTATESWITCHER_H
