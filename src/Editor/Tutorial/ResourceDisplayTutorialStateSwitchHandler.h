#ifndef CRESOURCEDISPLAYTUTORIALSTATESWITCHHANDLER_H
#define CRESOURCEDISPLAYTUTORIALSTATESWITCHHANDLER_H

#include "ITutorialStateSwitchHandler.h"
#include <QPointer>
#include <memory>

class CEditorResourceDisplayWidget;
namespace Ui {
  class CEditorResourceDisplayWidget;
}

class CResourceDisplayTutorialStateSwitchHandler : public ITutorialStateSwitchHandler
{
public:
  CResourceDisplayTutorialStateSwitchHandler(QPointer<CEditorResourceDisplayWidget> pParentWidget,
                                             const std::shared_ptr<Ui::CEditorResourceDisplayWidget>& spUi);
  ~CResourceDisplayTutorialStateSwitchHandler() override;

  void OnResetStates() override;
  void OnStateSwitch(ETutorialState newState, ETutorialState oldstate) override;

private:
  QPointer<CEditorResourceDisplayWidget>            m_pParentWidget;
  std::shared_ptr<Ui::CEditorResourceDisplayWidget> m_spUi;
};

#endif // CRESOURCEDISPLAYTUTORIALSTATESWITCHHANDLER_H
