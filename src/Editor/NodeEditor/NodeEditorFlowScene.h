#ifndef CNODEEDITORFLOWSCENE_H
#define CNODEEDITORFLOWSCENE_H

#include "Systems/Nodes/FlowScene.h"

#include <QPointer>
#include <QUndoStack>

class CNodeEditorFlowScene : public CFlowScene
{
  const static char* c_sUndoRedoingOperation;

public:
  CNodeEditorFlowScene(std::shared_ptr<QtNodes::DataModelRegistry> spRegistry,
                       QObject* pParent = Q_NULLPTR);
  ~CNodeEditorFlowScene() override;

  void SetUndoStack(QPointer<QUndoStack> pUndoStack);
  void SetUndoOperationInProgress(bool bValue) { m_bUndoRedoOperationInProgress = bValue; }
  QPointer<QUndoStack> UndoStack();

protected slots:
  void SlotConnectionCreated(QtNodes::Connection const &c);
  void SlotConnectionDeleted(QtNodes::Connection const &c);
  void SlotNodeCreated(QtNodes::Node& node);
  void SlotNodePlaced(QtNodes::Node& node);
  void SlotNodeMoved(QtNodes::Node& node, const QPointF& newPosition);

protected:
  void NodeCreatedImpl(QtNodes::Node& node) override;

private:
  QPointer<QUndoStack>     m_pUndoStack;
  bool                     m_bUndoRedoOperationInProgress;
};

//----------------------------------------------------------------------------------------
//
class CSceneUndoOperationLocker
{
public:
  CSceneUndoOperationLocker(QPointer<CNodeEditorFlowScene> pScene) : m_pScene(pScene)
  {
    Relock();
  }
  ~CSceneUndoOperationLocker()
  {
    Unlock();
  }

  void Relock() { m_pScene->SetUndoOperationInProgress(true); }
  void Unlock() { m_pScene->SetUndoOperationInProgress(false); }
private:
  QPointer<CNodeEditorFlowScene> m_pScene;
};

#endif // CNODEEDITORFLOWSCENE_H
