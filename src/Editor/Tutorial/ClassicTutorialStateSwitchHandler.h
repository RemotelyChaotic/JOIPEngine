#ifndef CMAINSCREENTUTORIALSTATESWITCHER_H
#define CMAINSCREENTUTORIALSTATESWITCHER_H

#include "TutorialStateSwitchHandlerMainBase.h"
#include <QPointer>
#include <memory>

class CEditorLayoutClassic;

class CClassicTutorialStateSwitchHandler : public CTutorialStateSwitchHandlerMainBase
{
  Q_OBJECT

public:
  CClassicTutorialStateSwitchHandler(QPointer<CEditorLayoutClassic> pParentWidget,
                                     QPointer<CEditorTutorialOverlay> pTutorialOverlay);
  ~CClassicTutorialStateSwitchHandler() override;

protected:
  void OnStateSwitchImpl(ETutorialState newState, ETutorialState oldState) override;

private slots:
  void SlotSwitchLeftPanel(qint32 iNewIndex);
  void SlotSwitchRightPanel(qint32 iNewIndex);
  void SlotRightPanelSwitched(qint32 iNewIndex);

private:
  QPointer<CEditorLayoutClassic>             m_ParentWidget;
};

#endif // CMAINSCREENTUTORIALSTATESWITCHER_H
