#include "NodeEditorFlowScene.h"
#include "CommandConnectionAdded.h"
#include "CommandConnectionRemoved.h"
#include "CommandNodeAdded.h"
#include "CommandNodeMoved.h"
#include "NodeEditorGraphicsObjectProvider.h"
#include "UndoStackAwareModel.h"

#include "Editor/EditorWidgetTypes.h"

#include "Systems/Nodes/FlowView.h"

using QtNodes::FlowScene;

const char* CNodeEditorFlowScene::c_sUndoRedoingOperation = "UndoRedoingOperation";

CNodeEditorFlowScene::CNodeEditorFlowScene(std::shared_ptr<QtNodes::DataModelRegistry> spRegistry,
                                           QObject* pParent) :
    CFlowScene(spRegistry, std::make_unique<CNodeEditorGraphicsObjectProvider>(), pParent),
  m_pUndoStack(nullptr),
  m_bUndoRedoOperationInProgress(false)
{
  connect(this, &FlowScene::connectionCreated, this, &CNodeEditorFlowScene::SlotConnectionCreated, Qt::DirectConnection);
  connect(this, &FlowScene::connectionDeleted, this, &CNodeEditorFlowScene::SlotConnectionDeleted, Qt::DirectConnection);
  connect(this, &FlowScene::nodeCreated, this, &CNodeEditorFlowScene::SlotNodeCreated, Qt::DirectConnection);
  connect(this, &FlowScene::nodePlaced, this, &CNodeEditorFlowScene::SlotNodePlaced, Qt::DirectConnection);
  connect(this, &FlowScene::nodeMoved, this, &CNodeEditorFlowScene::SlotNodeMoved, Qt::DirectConnection);
}

CNodeEditorFlowScene::~CNodeEditorFlowScene() = default;

//----------------------------------------------------------------------------------------
//
void CNodeEditorFlowScene::SetUndoStack(QPointer<QUndoStack> pUndoStack)
{
  if (nullptr != m_pUndoStack) { delete m_pUndoStack; }
  m_pUndoStack = pUndoStack;
}

//----------------------------------------------------------------------------------------
//
QPointer<QUndoStack> CNodeEditorFlowScene::UndoStack()
{
  return m_pUndoStack;
}

//----------------------------------------------------------------------------------------
//
void CNodeEditorFlowScene::SlotConnectionCreated(QtNodes::Connection const &c)
{
  if (nullptr != UndoStack() &&
      !m_bUndoRedoOperationInProgress &&
      !m_bLoading)
  {
    UndoStack()->push(new CCommandConnectionAdded(this, c.id(), c.save()));
  }
}

//----------------------------------------------------------------------------------------
//
void CNodeEditorFlowScene::SlotConnectionDeleted(QtNodes::Connection const &c)
{
  if (nullptr != UndoStack() &&
      !m_bUndoRedoOperationInProgress &&
      !m_bLoading)
  {
    UndoStack()->push(new CCommandConnectionRemoved(this, c.id(), c.save()));
  }
}

//----------------------------------------------------------------------------------------
//
void CNodeEditorFlowScene::SlotNodeCreated(QtNodes::Node& node)
{
  if (m_bLoading) { return; }
  NodeCreatedImpl(node);
}

//----------------------------------------------------------------------------------------
//
void CNodeEditorFlowScene::SlotNodePlaced(QtNodes::Node& node)
{
  node.nodeGraphicsObject().setProperty(editor::c_sPropertyOldValue, node.nodeGraphicsObject().pos());
}

//----------------------------------------------------------------------------------------
//
void CNodeEditorFlowScene::SlotNodeMoved(QtNodes::Node& node, const QPointF& newPosition)
{
  if (!m_bLoading && !m_bUndoRedoOperationInProgress)
  {
    QPointF oldPos =
        node.nodeGraphicsObject().property(editor::c_sPropertyOldValue).value<QPointF>();
    if (oldPos.x() != newPosition.x() && oldPos.y() != newPosition.y() &&
        nullptr != UndoStack() && views().size() > 0)
    {
      for (auto pView : views())
      {
        if (CFlowView* pViewCasted = dynamic_cast<CFlowView*>(pView))
        {
          UndoStack()->push(new CCommandNodeMoved(pViewCasted, node.id(),
                                                  oldPos, newPosition));
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CNodeEditorFlowScene::NodeCreatedImpl(QtNodes::Node& node)
{
  if (auto pTypeUndoAware = dynamic_cast<CUndoStackAwareModel*>(node.nodeDataModel()))
  {
    pTypeUndoAware->SetUndoStack(m_pUndoStack);
  }
  if (auto pBaseModel = dynamic_cast<CNodeModelBase*>(node.nodeDataModel()))
  {
    pBaseModel->SetNodeContext(node.id(), this);
  }
}
