#ifndef CNODEMODELBASE_H
#define CNODEMODELBASE_H

#include <nodes/NodeDataModel>

#include <QPointer>
#include <QUuid>

class CFlowScene;

using QtNodes::NodeDataModel;

class CNodeModelBase : public NodeDataModel
{
  Q_OBJECT

public:
  enum EDebugState
  {
    eNotDebugged,
    eDebugPassive,
    eDebugActive
  };

  CNodeModelBase();
  ~CNodeModelBase() override;

  EDebugState DebugState() const { return m_debugState; }

  void SetNodeContext(const QUuid& id, QPointer<CFlowScene> pScene);
  void SetDebuggState(EDebugState state);

protected:
  QUuid m_id;
  QPointer<CFlowScene> m_pScene;
  EDebugState m_debugState = EDebugState::eNotDebugged;
};

#endif // CNODEMODELBASE_H
