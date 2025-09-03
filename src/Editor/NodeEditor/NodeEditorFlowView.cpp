#include "NodeEditorFlowView.h"
#include "CommandNodeAdded.h"
#include "CommandNodeRemoved.h"
#include "NodeEditorFlowScene.h"

#include <QDebug>
#include <QKeyEvent>
#include <QHeaderView>
#include <QLineEdit>
#include <QMenu>
#include <QTreeWidget>
#include <QWidgetAction>

using QtNodes::FlowView;
using QtNodes::Node;

CNodeEditorFlowView::CNodeEditorFlowView(QWidget* pParent) :
  CFlowView(pParent),
  m_pUndoStack(new QUndoStack(this))
{
}
CNodeEditorFlowView::CNodeEditorFlowView(CNodeEditorFlowScene* pScene, QWidget* pParent) :
  CFlowView(pScene, pParent),
  m_pUndoStack(new QUndoStack(this))
{
}

CNodeEditorFlowView::~CNodeEditorFlowView() {}

//----------------------------------------------------------------------------------------
//
void CNodeEditorFlowView::SetSceneImpl(CFlowScene* pScene)
{
  if (CNodeEditorFlowScene* pSceneCasted = dynamic_cast<CNodeEditorFlowScene*>(pScene))
  {
    pSceneCasted->SetUndoStack(m_pUndoStack);
  }
}

//----------------------------------------------------------------------------------------
//
void CNodeEditorFlowView::SetUndoStack(QPointer<QUndoStack> pUndoStack)
{
  if (nullptr != m_pUndoStack) { delete m_pUndoStack; }
  m_pUndoStack = pUndoStack;
  if (CNodeEditorFlowScene* pScene = dynamic_cast<CNodeEditorFlowScene*>(scene()))
  {
    pScene->SetUndoStack(m_pUndoStack);
  }
}

//----------------------------------------------------------------------------------------
//
QPointer<QUndoStack> CNodeEditorFlowView::UndoStack()
{
  return m_pUndoStack;
}

//----------------------------------------------------------------------------------------
//
void CNodeEditorFlowView::OpenContextMenuAt(const QPoint& localPoint, const QPoint& createPoint)
{
  if (m_bReadOnly) { return; }
  CFlowView::OpenContextMenuAt(localPoint, createPoint);
}

//----------------------------------------------------------------------------------------
//
void CNodeEditorFlowView::SlotClearSelectionTriggered()
{

}

//----------------------------------------------------------------------------------------
//
void CNodeEditorFlowView::SlotDeleteTriggered()
{
  if (nullptr != scene())
  {
    // HACK HACK HACK: since ConnectionGraphicsObject is private, we check the class name
    // and iterate over all connections. We then get the connectionGraphicsObject of those and just
    // cast them to graphicsitems to compare the pointer vlues
    // TODO: Change library, so we have a getGraphicsObject method that returns a Qt-Object
    // instead of a private type
    const auto& mapConnecitons = scene()->connections();
    for (QGraphicsItem* pItem : scene()->selectedItems())
    {
      QGraphicsObject* pGraphicsObj = qgraphicsitem_cast<QGraphicsObject*>(pItem);
      if (nullptr != pGraphicsObj && pGraphicsObj->metaObject()->className() ==
                                         QString("ConnectionGraphicsObject"))
      {
        for (auto& connectionPair : mapConnecitons)
        {
          if (&reinterpret_cast<QGraphicsObject&>(connectionPair.second->getConnectionGraphicsObject()) ==
              pGraphicsObj)
          {
            scene()->deleteConnection(*connectionPair.second);
          }
        }
      }
    }

    std::vector<QUuid> vIds;
    for (Node* pNode : scene()->selectedNodes())
    {
      vIds.push_back(pNode->id());
    }

    if (nullptr != m_pUndoStack)
    {
      UndoStack()->push(new CCommandNodesRemoved(this, vIds, UndoStack()));
    }
    else
    {
      assert(false && "QUndoStack must never be null.");
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CNodeEditorFlowView::keyPressEvent(QKeyEvent *event)
{
  switch (event->key())
  {
    case Qt::Key_V:
      if (event->modifiers() & Qt::ControlModifier)
      {
        if (auto pScene = dynamic_cast<CNodeEditorFlowScene*>(Scene()))
        {
          pScene->SetUndoOperationInProgress(true);
          QtNodes::FlowView::keyPressEvent(event);
          pScene->SetUndoOperationInProgress(false);
        }
        return;
      }
      break;

    default:
      break;
  }

  QtNodes::FlowView::keyPressEvent(event);
}

//----------------------------------------------------------------------------------------
//
void CNodeEditorFlowView::NodeAboutToBeCreated(const QString& modelName, const QPoint& localPoint)
{
  if (nullptr != m_pUndoStack)
  {
    UndoStack()->push(new CCommandNodeAdded(this, modelName, localPoint, UndoStack()));
  }
  else
  {
    assert(false && "QUndoStack must never be null.");
  }
}
