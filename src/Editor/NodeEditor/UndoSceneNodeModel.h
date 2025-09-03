#ifndef CUNDOSCENENODEMODEL_H
#define CUNDOSCENENODEMODEL_H

#include "UndoStackAwareModel.h"

#include "Systems/Nodes/SceneNodeModel.h"

class CUndoSceneNodeModel : public CSceneNodeModel,
                            public CUndoStackAwareModel
{
  Q_OBJECT
public:
  CUndoSceneNodeModel();
  ~CUndoSceneNodeModel() override;

protected:
  void SlotCanStartHereChangedImpl(bool bValue) override;
  void SlotNameChangedImpl(const QString& sName) override;
  void SlotLayoutChangedImpl(const QString& sName) override;
  void SlotScriptChangedImpl(const QString& sName) override;
  void SlotTitleResourceChangedImpl(const QString& sOld, const QString& sNew) override;

  void UndoRestore(QJsonObject const& p) override;
  void OnUndoStackSet() override {}

  bool                             m_bIsInUndoOperation = false;
};

//----------------------------------------------------------------------------------------
//
class CUndoSceneNodeModelWithWidget : public CSceneNodeModelWithWidget,
                                      public CUndoStackAwareModel
{
  Q_OBJECT
public:
  CUndoSceneNodeModelWithWidget();
  ~CUndoSceneNodeModelWithWidget() override;

protected:
  void SlotCanStartHereChangedImpl(bool bValue) override;
  void SlotNameChangedImpl(const QString& sName) override;
  void SlotLayoutChangedImpl(const QString& sName) override;
  void SlotScriptChangedImpl(const QString& sName) override;
  void SlotTitleResourceChangedImpl(const QString& sOld, const QString& sNew) override;

  void UndoRestore(QJsonObject const& p) override;
  void OnUndoStackSet() override;

  bool                             m_bIsInUndoOperation = false;
};

#endif // CUNDOSCENENODEMODEL_H
