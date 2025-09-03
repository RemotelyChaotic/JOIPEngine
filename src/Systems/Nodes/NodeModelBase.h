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
  CNodeModelBase();
  ~CNodeModelBase() override;

  void SetNodeContext(const QUuid& id, QPointer<CFlowScene> pScene);

protected:
  QUuid m_id;
  QPointer<CFlowScene> m_pScene;
};

#endif // CNODEMODELBASE_H
