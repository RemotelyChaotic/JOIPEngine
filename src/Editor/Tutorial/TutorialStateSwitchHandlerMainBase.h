#ifndef CTUTORIALSTATESWITCHHANDLERMAINBASE_H
#define CTUTORIALSTATESWITCHHANDLERMAINBASE_H

#include "ITutorialStateSwitchHandler.h"
#include <QPointer>
#include <memory>

class CEditorTutorialOverlay;
class CJsonInstructionSetParser;
class CJsonInstructionSetRunner;

class CTutorialStateSwitchHandlerMainBase : public QObject, public ITutorialStateSwitchHandler
{
  Q_OBJECT

public:
  CTutorialStateSwitchHandlerMainBase(QPointer<CEditorTutorialOverlay> pTutorialOverlay,
                                      const QString& sTutorial);
  ~CTutorialStateSwitchHandlerMainBase() override;

  void OnResetStates() override;
  void OnStateSwitch(ETutorialState newState, ETutorialState oldState) override final;

protected slots:
  void SlotOverlayNextInstructionTriggered();

protected:
  virtual void OnStateSwitchImpl(ETutorialState, ETutorialState) {};

  std::unique_ptr<CJsonInstructionSetParser> m_spTutorialParser;
  std::shared_ptr<CJsonInstructionSetRunner> m_spTutorialRunner;
  QPointer<CEditorTutorialOverlay>           m_pTutorialOverlay;
  ETutorialState                             m_currentState;
};

#endif // CTUTORIALSTATESWITCHHANDLERMAINBASE_H
