#ifndef RESOURCETREEITEM_H
#define RESOURCETREEITEM_H

#include "Backend/Resource.h"
#include <QVariant>
#include <QVector>

BETTER_ENUM(EResourceTreeItemType, qint32,
            eRoot      = 0,
            eCategory  = 1,
            eFolder    = 2,
            eResource  = 3);

class CResourceTreeItem
{
public:
  explicit CResourceTreeItem(EResourceTreeItemType type, const QString& sLabel = QString(),
    tspResource spResource = nullptr, CResourceTreeItem* pParentItem = nullptr);
  ~CResourceTreeItem();

  void AppendChild(CResourceTreeItem* pChild);

  CResourceTreeItem* Child(qint32 iRow);
  qint32 ChildCount() const;
  qint32 ColumnCount() const;
  QVariant Data(qint32 iColumn) const;
  QVariant HeaderData(qint32 iColumn) const;
  qint32 Row() const;
  CResourceTreeItem* ParentItem();

private:
  CResourceTreeItem*          m_pParentItem;
  QVector<CResourceTreeItem*> m_vpChildItems;
  EResourceTreeItemType       m_type;
  QString                     m_sLabel;
  tspResource                 m_spResource;
};

#endif // RESOURCETREEITEM_H
