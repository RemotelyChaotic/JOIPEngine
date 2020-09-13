#ifndef CSCENENODEWIDGETTUTORIALSTATESWITCHHANDLER_H
#define CSCENENODEWIDGETTUTORIALSTATESWITCHHANDLER_H

#include "ITutorialStateSwitchHandler.h"
#include <QPointer>
#include <memory>

class CEditorSceneNodeWidget;
namespace Ui {
  class CEditorSceneNodeWidget;
}


class CSceneNodeWidgetTutorialStateSwitchHandler : public ITutorialStateSwitchHandler
{
public:
  CSceneNodeWidgetTutorialStateSwitchHandler(QPointer<CEditorSceneNodeWidget> pParentWidget,
                                             const std::shared_ptr<Ui::CEditorSceneNodeWidget>& spUi);
  ~CSceneNodeWidgetTutorialStateSwitchHandler() override;

  void OnResetStates() override;
  void OnStateSwitch(ETutorialState newState, ETutorialState oldstate) override;

private:
  QPointer<CEditorSceneNodeWidget>                  m_pParentWidget;
  std::shared_ptr<Ui::CEditorSceneNodeWidget>       m_spUi;
};

#endif // CSCENENODEWIDGETTUTORIALSTATESWITCHHANDLER_H
