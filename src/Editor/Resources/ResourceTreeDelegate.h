#ifndef CRESOURCETREEDELEGATE_H
#define CRESOURCETREEDELEGATE_H

#include <QPixmap>
#include <QStyledItemDelegate>

class CResourceTreeDelegate  : public QStyledItemDelegate
{
  Q_OBJECT

public:
  CResourceTreeDelegate(QObject* pParent = nullptr);
  ~CResourceTreeDelegate() override;

protected:
  void paint(QPainter* pPainter, const QStyleOptionViewItem& option,
             const QModelIndex& index) const override;

  QPixmap m_warnIcon;
};

#endif // CRESOURCETREEDELEGATE_H
