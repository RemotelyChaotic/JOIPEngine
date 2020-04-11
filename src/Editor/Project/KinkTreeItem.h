#ifndef KINKTREEITEM_H
#define KINKTREEITEM_H

#include "Systems/Kink.h"
#include <enum.h>
#include <QVariant>
#include <QVector>

class CKinkTreeItem
{
public:
  explicit CKinkTreeItem(QString sKinkCategory = QString(), tspKink spKink = nullptr, CKinkTreeItem* pParentItem = nullptr);
  ~CKinkTreeItem();

  void AppendChild(CKinkTreeItem* pChild);

  CKinkTreeItem* Child(qint32 iRow);
  qint32 ChildCount() const;
  QVariant Data(qint32 iRole) const;
  qint32 Row() const;
  CKinkTreeItem* Parent();

private:
  QVector<CKinkTreeItem*> m_vpChildItems;
  CKinkTreeItem*          m_pParentItem;
  tspKink                 m_spKink;
  QString                 m_sKinkCategory;
};

#endif // KINKTREEITEM_H
