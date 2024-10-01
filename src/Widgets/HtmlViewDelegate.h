#ifndef CHTMLVIEWDELEGATE_H
#define CHTMLVIEWDELEGATE_H

#include <QStyledItemDelegate>
#include <QTreeView>

class CHtmlViewDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  CHtmlViewDelegate(QTreeView* pTree);

protected:
  void paint(QPainter* pPainter,
             const QStyleOptionViewItem& option,
             const QModelIndex& index) const override;
  QSize sizeHint(
      const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

#endif // CHTMLVIEWDELEGATE_H
