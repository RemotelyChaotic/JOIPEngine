#ifndef CMAINSCREENTUTORIALSTATESWITCHER_H
#define CMAINSCREENTUTORIALSTATESWITCHER_H

#include "ITutorialStateSwitchHandler.h"
#include <QPointer>
#include <memory>

class CEditorMainScreen;
namespace Ui {
  class CEditorMainScreen;
}

class CMainScreenTutorialStateSwitchHandler : public ITutorialStateSwitchHandler
{
public:
  CMainScreenTutorialStateSwitchHandler(QPointer<CEditorMainScreen> pParentWidget,
                                        const std::shared_ptr<Ui::CEditorMainScreen>& spUi);
  ~CMainScreenTutorialStateSwitchHandler() override;

  void OnResetStates() override;
  void OnStateSwitch(ETutorialState newState, ETutorialState oldstate) override;

private:
  std::shared_ptr<Ui::CEditorMainScreen>   m_spUi;
  QPointer<CEditorMainScreen>              m_ParentWidget;
};

#endif // CMAINSCREENTUTORIALSTATESWITCHER_H
