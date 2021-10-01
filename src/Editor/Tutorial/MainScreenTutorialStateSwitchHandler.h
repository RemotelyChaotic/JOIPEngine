#ifndef CMAINSCREENTUTORIALSTATESWITCHER_H
#define CMAINSCREENTUTORIALSTATESWITCHER_H

#include "ITutorialStateSwitchHandler.h"
#include <QPointer>
#include <memory>

class CEditorTutorialOverlay;
class CJsonInstructionSetParser;
class CJsonInstructionSetRunner;
class CEditorLayoutClassic;

class CMainScreenTutorialStateSwitchHandler : public QObject, public ITutorialStateSwitchHandler
{
  Q_OBJECT

public:
  CMainScreenTutorialStateSwitchHandler(QPointer<CEditorLayoutClassic> pParentWidget,
                                        QPointer<CEditorTutorialOverlay> pTutorialOverlay);
  ~CMainScreenTutorialStateSwitchHandler() override;

  void OnResetStates() override;
  void OnStateSwitch(ETutorialState newState, ETutorialState oldState) override;

private slots:
  void SlotSwitchLeftPanel(qint32 iNewIndex);
  void SlotSwitchRightPanel(qint32 iNewIndex);
  void SlotRightPanelSwitched(qint32 iNewIndex);
  void SlotOverlayNextInstructionTriggered();

private:
  std::unique_ptr<CJsonInstructionSetParser> m_spTutorialParser;
  std::shared_ptr<CJsonInstructionSetRunner> m_spTutorialRunner;
  QPointer<CEditorLayoutClassic>             m_ParentWidget;
  QPointer<CEditorTutorialOverlay>           m_pTutorialOverlay;
  ETutorialState                             m_currentState;
};

#endif // CMAINSCREENTUTORIALSTATESWITCHER_H
