#ifndef CEDITORNODEMODELBASE_H
#define CEDITORNODEMODELBASE_H

#include "IUndoStackAwareModel.h"
#include <nodes/NodeDataModel>
#include <QUuid>

class CFlowScene;

using QtNodes::NodeDataModel;

class CEditorNodeModelBase : public NodeDataModel, public IUndoStackAwareModel
{
  Q_OBJECT

public:
  CEditorNodeModelBase();
  ~CEditorNodeModelBase() override;

  void SetNodeContext(const QUuid& id, QPointer<CFlowScene> pScene);

  void UndoRestore(const QJsonObject& obj) override;

protected:
  QUuid m_id;
  QPointer<CFlowScene> m_pScene;
};

#endif // CEDITORNODEMODELBASE_H
