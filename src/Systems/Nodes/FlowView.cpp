#include "FlowView.h"
#include "CommandConnectionAdded.h"
#include "CommandConnectionRemoved.h"
#include "CommandNodeAdded.h"
#include "CommandNodeEdited.h"
#include "CommandNodeMoved.h"
#include "CommandNodeRemoved.h"
#include "FlowScene.h"

#include "Widgets/SearchWidget.h"

#include <nodes/Connection>
#include <nodes/FlowScene>
#include <nodes/NodeGeometry>
#include <nodes/Node>

#include <QContextMenuEvent>
#include <QDebug>
#include <QGraphicsItem>
#include <QHeaderView>
#include <QLineEdit>
#include <QMenu>
#include <QTreeWidget>
#include <QUndoStack>
#include <QWidgetAction>

using QtNodes::FlowView;
using QtNodes::Node;

CFlowView::CFlowView(QWidget* pParent) :
  FlowView(pParent),
  m_bReadOnly(false),
  m_contextMenuItemVisibility(),
  m_pUndoStack(new QUndoStack(this))
{
  setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
}

CFlowView::CFlowView(CFlowScene* pScene, QWidget* pParent) :
  FlowView(pScene, pParent),
  m_bReadOnly(false),
  m_contextMenuItemVisibility(),
  m_pUndoStack(new QUndoStack(this))
{
  setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
}

CFlowView::~CFlowView() {}

//----------------------------------------------------------------------------------------
//
void CFlowView::SetScene(CFlowScene* pScene)
{
  if (nullptr != pScene)
  {
    pScene->SetUndoStack(m_pUndoStack);
  }
  FlowView::setScene(pScene);
  clearSelectionAction()->disconnect();
  deleteSelectionAction()->disconnect();
  connect(clearSelectionAction(), &QAction::triggered, this, &CFlowView::SlotClearSelectionTriggered);
  connect(deleteSelectionAction(), &QAction::triggered, this, &CFlowView::SlotDeleteTriggered);
}

//----------------------------------------------------------------------------------------
//
CFlowScene* CFlowView::Scene()
{
  return dynamic_cast<CFlowScene*>(scene());
}

//----------------------------------------------------------------------------------------
//
void CFlowView::SetUndoStack(QPointer<QUndoStack> pUndoStack)
{
  if (nullptr != m_pUndoStack) { delete m_pUndoStack; }
  m_pUndoStack = pUndoStack;
  if (CFlowScene* pScene = dynamic_cast<CFlowScene*>(scene()))
  {
    pScene->SetUndoStack(m_pUndoStack);
  }
}

//----------------------------------------------------------------------------------------
//
QPointer<QUndoStack> CFlowView::UndoStack()
{
  return m_pUndoStack;
}

//----------------------------------------------------------------------------------------
//
void CFlowView::FitAllNodesInView()
{
  resetTransform();
  scale(0.8, 0.8);
  centerOn(0,0);

  CFlowScene* pScene = Scene();
  if (nullptr != pScene)
  {
    QRectF nodeRects;
    const auto& nodes = pScene->nodes();
    for (auto it = nodes.begin(); nodes.end() != it; ++it)
    {
      nodeRects = nodeRects.united({it->second->nodeGraphicsObject().pos(),
                                    QSizeF{static_cast<qreal>(it->second->nodeGeometry().width()),
                                           static_cast<qreal>(it->second->nodeGeometry().height())}});
    }
    setSceneRect(nodeRects);
    update();
  }
}

//----------------------------------------------------------------------------------------
//
// Taken from FlowView::contextMenuEvent and modified slightly
void CFlowView::OpenContextMenuAt(const QPoint& localPoint, const QPoint& createPoint)
{
  if (m_bReadOnly) { return; }

  QMenu modelMenu;

  auto skipText = QStringLiteral("skip me");

  //Add filterbox to the context menu
  auto *txtBox = new CSearchWidget(&modelMenu);

  auto *txtBoxAction = new QWidgetAction(&modelMenu);
  txtBoxAction->setDefaultWidget(txtBox);

  modelMenu.addAction(txtBoxAction);

  //Add result treeview to the context menu
  auto *treeView = new QTreeWidget(&modelMenu);
  treeView->header()->close();

  auto *treeViewAction = new QWidgetAction(&modelMenu);
  treeViewAction->setDefaultWidget(treeView);

  modelMenu.addAction(treeViewAction);

  QMap<QString, QTreeWidgetItem*> topLevelItems;
  for (auto const &cat : scene()->registry().categories())
  {
    auto item = new QTreeWidgetItem(treeView);
    item->setText(0, cat);
    item->setData(0, Qt::UserRole, skipText);
    topLevelItems[cat] = item;
  }

  for (auto const &assoc : scene()->registry().registeredModelsCategoryAssociation())
  {
    auto parent = topLevelItems[assoc.second];
    auto item   = new QTreeWidgetItem(parent);
    item->setText(0, assoc.first);
    item->setData(0, Qt::UserRole, assoc.first);
    if (IsModelHiddenInContextMenu(assoc.first))
    {
      item->setFlags(item->flags() & ~Qt::ItemFlag::ItemIsEnabled);
    }
  }

  treeView->expandAll();

  connect(treeView, &QTreeWidget::itemClicked, treeView, [&](QTreeWidgetItem *item, int)
  {
    QString modelName = item->data(0, Qt::UserRole).toString();

    if (modelName == skipText)
    {
      return;
    }

    if (nullptr != m_pUndoStack)
    {
      UndoStack()->push(new CCommandNodeAdded(this, modelName, localPoint, UndoStack()));
    }
    else
    {
      assert(false && "QUndoStack must never be null.");
    }

    modelMenu.close();
  });

  //Setup filtering
  connect(txtBox, &CSearchWidget::SignalFilterChanged, txtBox, [&](const QString &text)
  {
    for (auto& topLvlItem : topLevelItems)
    {
      for (int i = 0; i < topLvlItem->childCount(); ++i)
      {
        auto child = topLvlItem->child(i);
        auto modelName = child->data(0, Qt::UserRole).toString();
        const bool match = (modelName.contains(text, Qt::CaseInsensitive));
        child->setHidden(!match);
      }
    }
  });

  // make sure the text box gets focus so the user doesn't have to click on it
  txtBox->setFocus();

  modelMenu.exec(createPoint);
}

//----------------------------------------------------------------------------------------
//
bool CFlowView::IsReadOnly() { return m_bReadOnly; }

//----------------------------------------------------------------------------------------
//
void CFlowView::SetReadOnly(bool bReadOnly)
{
  if (m_bReadOnly != bReadOnly)
  {
    m_bReadOnly = bReadOnly;
    CFlowScene* pScene = Scene();
    if (nullptr != pScene)
    {
      pScene->iterateOverNodes([&bReadOnly](QtNodes::Node* pNode) {
        if (nullptr != pNode)
        {
          QtNodes::NodeGraphicsObject& pObj = pNode->nodeGraphicsObject();
          pObj.setEnabled(!bReadOnly);
        }
      });
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CFlowView::SetModelHiddenInContextMenu(const QString& sId, bool bHidden)
{
  m_contextMenuItemVisibility[sId] = bHidden;
}

//----------------------------------------------------------------------------------------
//
bool CFlowView::IsModelHiddenInContextMenu(const QString& sId)
{
  auto it = m_contextMenuItemVisibility.find(sId);
  if (m_contextMenuItemVisibility.end() != it)
  {
    return it->second;
  }
  else
  {
    return false;
  }
}

//----------------------------------------------------------------------------------------
//
void CFlowView::SlotClearSelectionTriggered()
{

}

//----------------------------------------------------------------------------------------
//
void CFlowView::SlotDeleteTriggered()
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
void CFlowView::contextMenuEvent(QContextMenuEvent* pEvent)
{
  if (!m_bReadOnly)
  {
    if (itemAt(pEvent->pos()))
    {
      QGraphicsView::contextMenuEvent(pEvent);
      return;
    }

    OpenContextMenuAt(pEvent->pos(), pEvent->globalPos());
  }
}

//----------------------------------------------------------------------------------------
//
void CFlowView::keyPressEvent(QKeyEvent *event)
{
  switch (event->key())
  {
    case Qt::Key_V:
       if (event->modifiers() & Qt::ControlModifier)
       {
         Scene()->SetUndoOperationInProgress(true);
         QtNodes::FlowView::keyPressEvent(event);
         Scene()->SetUndoOperationInProgress(false);
         return;
       }
       break;

    default:
      break;
  }

  QtNodes::FlowView::keyPressEvent(event);
}
