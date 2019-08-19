#include "FlowView.h"

#include <nodes/FlowScene>
#include <nodes/NodeGeometry>
#include <nodes/Node>

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
  FlowView(pParent)
{

}

CFlowView::CFlowView(FlowScene* pScene, QWidget* pParent) :
  FlowView(pScene, pParent)
{
}

CFlowView::~CFlowView() {}

//----------------------------------------------------------------------------------------
//
void CFlowView::OpenContextMenuAt(const QPoint& localPoint, const QPoint& createPoint)
{
  QMenu modelMenu;

  auto skipText = QStringLiteral("skip me");

  //Add filterbox to the context menu
  auto* txtBox = new QLineEdit(&modelMenu);

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
  }

  treeView->expandAll();

  connect(treeView, &QTreeWidget::itemClicked, [&](QTreeWidgetItem *item, int)
  {
    QString modelName = item->data(0, Qt::UserRole).toString();

    if (modelName == skipText)
    {
      return;
    }

    auto type = scene()->registry().create(modelName);

    if (type)
    {
      auto& node = scene()->createNode(std::move(type));

      QPoint finalCreatPoint = createPoint;
      if (createPoint.isNull())
      {
        finalCreatPoint = localPoint;
      }

      QPointF posView = this->mapToScene(finalCreatPoint);
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

  modelMenu.exec(mapToGlobal(localPoint));
}
