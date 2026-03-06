#ifndef CUNDOSUBFLOWNODEMODEL_H
#define CUNDOSUBFLOWNODEMODEL_H

#include "UndoStackAwareModel.h"

#include "Systems/Nodes/SubflowNodeModel.h"

class CUndoSubflowNodeModel : public CSubflowNodeModel,
                              public CUndoStackAwareModel
{
  Q_OBJECT

public:
  CUndoSubflowNodeModel();
  ~CUndoSubflowNodeModel() override;

protected:
  void SlotFlowChangedImpl(const QString& sName) override;
  void SlotNameChangedImpl(const QString& sName) override;

  void UndoRestore(QJsonObject const& p) override;
  void OnUndoStackSet() override {}

  bool                             m_bIsInUndoOperation = false;
};

//----------------------------------------------------------------------------------------
//
class CUndoSubflowNodeModelWithWidget : public CSubflowNodeModelWithWidget,
                                        public CUndoStackAwareModel
{
  Q_OBJECT
public:
  CUndoSubflowNodeModelWithWidget();
  ~CUndoSubflowNodeModelWithWidget() override;

protected:
  void SlotFlowChangedImpl(const QString& sName) override;
  void SlotNameChangedImpl(const QString& sName) override;

  void UndoRestore(QJsonObject const& p) override;
  void OnUndoStackSet() override {}

  bool                             m_bIsInUndoOperation = false;
};

#endif // CUNDOSUBFLOWNODEMODEL_H
