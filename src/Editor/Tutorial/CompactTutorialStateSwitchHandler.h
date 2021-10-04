#ifndef CCOMPACTTUTORIALSTATESWITCHHANDLER_H
#define CCOMPACTTUTORIALSTATESWITCHHANDLER_H

#include "TutorialStateSwitchHandlerMainBase.h"
#include <QPointer>
#include <memory>

class CEditorLayoutCompact;

class CCompactTutorialStateSwitchHandler : public CTutorialStateSwitchHandlerMainBase
{
  Q_OBJECT

public:
  CCompactTutorialStateSwitchHandler(QPointer<CEditorLayoutCompact> pParentWidget,
                                     QPointer<CEditorTutorialOverlay> pTutorialOverlay);
  ~CCompactTutorialStateSwitchHandler() override;

protected:
  void OnStateSwitchImpl(ETutorialState newState, ETutorialState oldState) override;

private slots:
  void SlotSwitchView(qint32 iView);
  void SlotViewSwitched(qint32 iView);

private:
  QPointer<CEditorLayoutCompact>             m_ParentWidget;
};

#endif // CCOMPACTTUTORIALSTATESWITCHHANDLER_H
