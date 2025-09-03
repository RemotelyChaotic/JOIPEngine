#include "FlowView.h"
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
  m_contextMenuItemVisibility()
{
  setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
}

CFlowView::CFlowView(CFlowScene* pScene, QWidget* pParent) :
  FlowView(pScene, pParent),
  m_bReadOnly(false),
  m_contextMenuItemVisibility()
{
  setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
}

CFlowView::~CFlowView() {}

//----------------------------------------------------------------------------------------
//
void CFlowView::SetScene(CFlowScene* pScene)
{
  SetSceneImpl(pScene);
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

    NodeAboutToBeCreated(modelName, localPoint);

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
