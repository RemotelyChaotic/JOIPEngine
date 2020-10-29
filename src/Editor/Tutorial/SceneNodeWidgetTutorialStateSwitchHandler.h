#ifndef CSCENENODEWIDGETTUTORIALSTATESWITCHHANDLER_H
#define CSCENENODEWIDGETTUTORIALSTATESWITCHHANDLER_H

#include "ITutorialStateSwitchHandler.h"
#include <QPointer>
#include <memory>

class CEditorSceneNodeWidget;
namespace QtNodes {
  class Node;
  class Connection;
}
namespace Ui {
  class CEditorSceneNodeWidget;
}

class CSceneNodeWidgetTutorialStateSwitchHandler : public QObject, public ITutorialStateSwitchHandler
{
  Q_OBJECT

public:
  CSceneNodeWidgetTutorialStateSwitchHandler(QPointer<CEditorSceneNodeWidget> pParentWidget,
                                             const std::shared_ptr<Ui::CEditorSceneNodeWidget>& spUi);
  ~CSceneNodeWidgetTutorialStateSwitchHandler() override;

  void OnResetStates() override;
  void OnStateSwitch(ETutorialState newState, ETutorialState oldstate) override;

protected slots:
  void SlotConnectionCreated(QtNodes::Connection const &c);
  void SlotConnectionCheck();
  void SlotNodeCreated(QtNodes::Node& node);
  void SlotResourceAdded(qint32 iProjId, const QString& sName);

protected:
  bool eventFilter(QObject* pObj, QEvent* pEvt) override;

private:
  void TriggerNextInstruction();

  QPointer<CEditorSceneNodeWidget>                  m_pParentWidget;
  std::shared_ptr<Ui::CEditorSceneNodeWidget>       m_spUi;
  ETutorialState                                    m_currentState;
  bool                                              m_bMenuOpenedForFirstTime;
  bool                                              m_bFirstStartNodeCreated;
  bool                                              m_bFirstEndNodeCreated;
  bool                                              m_bFirstDefaultNodeCreated;
  bool                                              m_bFirstDefaultNodeScriptAdded;
  bool                                              m_bCompleteConnectionCreated;
  QMetaObject::Connection                           m_nodeCreatedConnection;
  QMetaObject::Connection                           m_resourceAddedConnection;
  QMetaObject::Connection                           m_connectionAddedConnection;
};

#endif // CSCENENODEWIDGETTUTORIALSTATESWITCHHANDLER_H
