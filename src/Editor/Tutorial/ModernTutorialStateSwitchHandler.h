#ifndef CMODERNTUTORIALSTATESWITCHHANDLER_H
#define CMODERNTUTORIALSTATESWITCHHANDLER_H

#include "TutorialStateSwitchHandlerMainBase.h"
#include <QPointer>
#include <memory>

class CEditorLayoutModern;

class CModernTutorialStateSwitchHandler : public CTutorialStateSwitchHandlerMainBase
{
  Q_OBJECT

public:
  CModernTutorialStateSwitchHandler(QPointer<CEditorLayoutModern> pParentWidget,
                                    QPointer<CEditorTutorialOverlay> pTutorialOverlay);
 ~CModernTutorialStateSwitchHandler() override;

protected:
  void OnStateSwitchImpl(ETutorialState newState, ETutorialState oldState) override;

private slots:
  void SlotSwitchLeftPanel(qint32 iNewIndex);
  void SlotSwitchRightPanel(qint32 iNewIndex);
  void SlotRightPanelSwitched(qint32 iNewIndex);

private:
  QPointer<CEditorLayoutModern>             m_ParentWidget;
};

#endif // CMODERNTUTORIALSTATESWITCHHANDLER_H
