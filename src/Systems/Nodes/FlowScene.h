#ifndef CFLOWSCENE_H
#define CFLOWSCENE_H

#include <nodes/FlowScene>
#include <nodes/Node>

#include <QPointer>
#include <QUndoStack>

#include <type_traits>

class CNodeGraphicsObjectProvider;

class CFlowScene : public QtNodes::FlowScene
{
public:
  const static char* c_sUndoRedoingOperation;

  CFlowScene(std::shared_ptr<QtNodes::DataModelRegistry> spRegistry,
             std::shared_ptr<CNodeGraphicsObjectProvider> spGraphicsObjectProvider,
             QObject* pParent = Q_NULLPTR);
  ~CFlowScene() override;

  void SetUndoStack(QPointer<QUndoStack> pUndoStack);
  void SetUndoOperationInProgress(bool bValue) { m_bUndoRedoOperationInProgress = bValue; }
  QPointer<QUndoStack> UndoStack();

  // shadow
  QtNodes::Node& createNode(std::unique_ptr<QtNodes::NodeDataModel>&& dataModel);
  void loadFromMemory(const QByteArray& data);
  void loadFromObject(const QJsonObject& data);
  QtNodes::Node& restoreNode(QJsonObject const& nodeJson);

protected:
  void SlotConnectionCreated(QtNodes::Connection const &c);
  void SlotConnectionDeleted(QtNodes::Connection const &c);
  void SlotNodeCreated(QtNodes::Node& node);
  void SlotNodePlaced(QtNodes::Node& node);
  void SlotNodeMoved(QtNodes::Node& node, const QPointF& newPosition);

private:
  std::shared_ptr<CNodeGraphicsObjectProvider> m_spGraphicsObjectProvider;
  QPointer<QUndoStack>     m_pUndoStack;
  bool                     m_bUndoRedoOperationInProgress;
  bool                     m_bLoading;
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
