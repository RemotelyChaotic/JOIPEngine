#ifndef CCODEWIDGETTUTORIALSTATESWITCHHANDLER_H
#define CCODEWIDGETTUTORIALSTATESWITCHHANDLER_H

#include "ITutorialStateSwitchHandler.h"
#include <QPointer>
#include <memory>

class CEditorCodeWidget;
namespace Ui {
  class CEditorCodeWidget;
}

class CCodeWidgetTutorialStateSwitchHandler : public ITutorialStateSwitchHandler
{
public:
  CCodeWidgetTutorialStateSwitchHandler(QPointer<CEditorCodeWidget> pParentWidget,
                                        const std::shared_ptr<Ui::CEditorCodeWidget>& spUI);
  ~CCodeWidgetTutorialStateSwitchHandler() override;

  void OnResetStates() override;
  void OnStateSwitch(ETutorialState newState, ETutorialState oldstate) override;

private:
  QPointer<CEditorCodeWidget>            m_pParentWidget;
  std::shared_ptr<Ui::CEditorCodeWidget> m_spUi;
};

#endif // CCODEWIDGETTUTORIALSTATESWITCHHANDLER_H
