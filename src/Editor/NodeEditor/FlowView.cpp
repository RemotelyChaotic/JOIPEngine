#include "FlowView.h"

#include <nodes/FlowScene>
#include <nodes/NodeGeometry>
#include <nodes/Node>

#include <QContextMenuEvent>
#include <QDebug>
#include <QHeaderView>
#include <QLineEdit>
#include <QMenu>
#include <QTreeWidget>
#include <QWidgetAction>

using QtNodes::FlowView;
using QtNodes::FlowScene;
using QtNodes::Node;

CFlowView::CFlowView(QWidget* pParent) :
  FlowView(pParent),
  m_bReadOnly(false),
  m_contextMenuItemVisibility()
{

}

CFlowView::CFlowView(FlowScene* pScene, QWidget* pParent) :
  FlowView(pScene, pParent),
  m_bReadOnly(false),
  m_contextMenuItemVisibility()
{
}

CFlowView::~CFlowView() {}

//----------------------------------------------------------------------------------------
//
// Taken from FlowView::contextMenuEvent and modified slightly
void CFlowView::OpenContextMenuAt(const QPoint& localPoint, const QPoint& createPoint)
{
  if (m_bReadOnly) { return; }

  QMenu modelMenu;

  auto skipText = QStringLiteral("skip me");

  //Add filterbox to the context menu
  auto *txtBox = new QLineEdit(&modelMenu);

  txtBox->setPlaceholderText(QStringLiteral("Filter"));
  txtBox->setClearButtonEnabled(true);

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

  connect(treeView, &QTreeWidget::itemClicked, [&](QTreeWidgetItem *item, int)
  {
    QString modelName = item->data(0, Qt::UserRole).toString();

    if (modelName == skipText)
    {
      return;
    }

    if (IsModelHiddenInContextMenu(modelName))
    {
      return;
    }

    auto type = scene()->registry().create(modelName);

    if (type)
    {
      auto& node = scene()->createNode(std::move(type));

      QPointF posView = this->mapToScene(localPoint);

      node.nodeGraphicsObject().setPos(posView);

      scene()->nodePlaced(node);
    }
    else
    {
      qDebug() << "Model not found";
    }

    modelMenu.close();
  });

  //Setup filtering
  connect(txtBox, &QLineEdit::textChanged, [&](const QString &text)
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
    QtNodes::FlowScene* pScene = scene();
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
