#ifndef CFLOWSCENE_H
#define CFLOWSCENE_H

#include <nodes/FlowScene>
#include <QPointer>
#include <QUndoStack>

class CFlowScene : public QtNodes::FlowScene
{
public:
  const static char* c_sUndoRedoingOperation;

  CFlowScene(std::shared_ptr<QtNodes::DataModelRegistry> spRegistry,
             QObject* pParent = Q_NULLPTR);
  ~CFlowScene() override;

  void SetUndoStack(QPointer<QUndoStack> pUndoStack);
  void SetUndoOperationInProgress(bool bValue) { m_bUndoRedoOperationInProgress = bValue; }
  QPointer<QUndoStack> UndoStack();

protected:
  void SlotConnectionCreated(QtNodes::Connection const &c);
  void SlotConnectionDeleted(QtNodes::Connection const &c);
  void SlotNodeCreated(QtNodes::Node& node);
  void SlotNodePlaced(QtNodes::Node& node);
  void SlotNodeMoved(QtNodes::Node& node, const QPointF& newPosition);

private:
  QPointer<QUndoStack>     m_pUndoStack;
  bool                     m_bUndoRedoOperationInProgress;
};

//----------------------------------------------------------------------------------------
//
class CSceneUndoOperationLocker
{
public:
  CSceneUndoOperationLocker(QPointer<CFlowScene> pScene) : m_pScene(pScene)
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
  QPointer<CFlowScene> m_pScene;
};

#endif // CFLOWSCENE_H
