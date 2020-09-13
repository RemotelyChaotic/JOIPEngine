#ifndef CRESOURCETUTORIALSTATESWITCHHANDLER_H
#define CRESOURCETUTORIALSTATESWITCHHANDLER_H

#include "ITutorialStateSwitchHandler.h"
#include <QPointer>
#include <memory>

class CEditorResourceWidget;
namespace Ui {
  class CEditorResourceWidget;
}

class CResourceTutorialStateSwitchHandler : public ITutorialStateSwitchHandler
{
public:
  CResourceTutorialStateSwitchHandler(QPointer<CEditorResourceWidget> pParentWidget,
                                      const std::shared_ptr<Ui::CEditorResourceWidget>& spUi);
  ~CResourceTutorialStateSwitchHandler() override;

  void OnResetStates() override;
  void OnStateSwitch(ETutorialState newState, ETutorialState oldstate) override;

private:
  QPointer<CEditorResourceWidget>                  m_pParentWidget;
  std::shared_ptr<Ui::CEditorResourceWidget>       m_spUi;
};

#endif // CRESOURCETUTORIALSTATESWITCHHANDLER_H
