#ifndef CPROJECTSETTINGSTUTORIALSTATESWITCHHANDLER_H
#define CPROJECTSETTINGSTUTORIALSTATESWITCHHANDLER_H

#include "ITutorialStateSwitchHandler.h"
#include <QPointer>
#include <memory>

class CEditorProjectSettingsWidget;
namespace Ui {
  class CEditorProjectSettingsWidget;
}

class CProjectSettingsTutorialStateSwitchHandler : public ITutorialStateSwitchHandler
{
public:
  CProjectSettingsTutorialStateSwitchHandler(QPointer<CEditorProjectSettingsWidget> pParentWidget,
                                             const std::shared_ptr<Ui::CEditorProjectSettingsWidget>& spUi);
  ~CProjectSettingsTutorialStateSwitchHandler() override;

  void OnResetStates() override;
  void OnStateSwitch(ETutorialState newState, ETutorialState oldstate) override;

private:
  QPointer<CEditorProjectSettingsWidget>            m_pParentWidget;
  std::shared_ptr<Ui::CEditorProjectSettingsWidget> m_spUi;
};

#endif // CPROJECTSETTINGSTUTORIALSTATESWITCHHANDLER_H
