#include "ResourceTreeDelegate.h"
#include "ResourceTreeItemModel.h"

#include <QApplication>
#include <QPainter>

CResourceTreeDelegate::CResourceTreeDelegate(QObject* pParent) :
  QStyledItemDelegate(pParent),
  m_warnIcon(":/resources/style/img/WarningIcon.png")
{

}

CResourceTreeDelegate::~CResourceTreeDelegate()
{}

//----------------------------------------------------------------------------------------
//
void CResourceTreeDelegate::paint(
    QPainter* pPainter, const QStyleOptionViewItem& option,
    const QModelIndex& index) const
{
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);

  if (nullptr != index.model())
  {
    const QString sWarning =
        index.model()->data(index, CResourceTreeItemModel::eItemWarningRole).toString();

    const QWidget* pWidget = option.widget;
    QStyle* pStyle = pWidget ? pWidget->style() : QApplication::style();

    if (sWarning.isEmpty())
    {
      pStyle->drawControl(QStyle::CE_ItemViewItem, &opt, pPainter, pWidget);
    }
    else
    {
      QRect textRect = pStyle->proxy()->subElementRect(QStyle::SE_ItemViewItemText,
                                                      &opt, pWidget);
      QRect destRect = QRect(textRect.x() + textRect.width() - 4 - textRect.height(),
                             textRect.y(),
                             textRect.height(),
                             textRect.height());
      pStyle->drawControl(QStyle::CE_ItemViewItem, &opt, pPainter, pWidget);
      pStyle->drawItemPixmap(pPainter, destRect, Qt::AlignVCenter | Qt::AlignRight,
                             m_warnIcon.scaled(destRect.size()));
    }
  }
}
