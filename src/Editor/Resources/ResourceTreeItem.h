#ifndef RESOURCETREEITEM_H
#define RESOURCETREEITEM_H

#include "Systems/Resource.h"
#include <QVariant>
#include <QVector>
#include <optional>

BETTER_ENUM(EResourceTreeItemType, qint32,
            eRoot      = 0,
            eCategory  = 1,
            eFolder    = 2,
            eResource  = 3);

namespace resource_item
{
  const qint32 c_iColumnName    = 0;
  const qint32 c_iColumnType    = 1;
  const qint32 c_iColumnPath    = 2;

  const qint32 c_iNumColumns    = 3;

  // not shown
  const qint32 c_iColumnToolTip = 4;
  const qint32 c_iColumnLoadedID= 5;
  const qint32 c_iColumnWarning = 6;

  const qint32 c_iNumItems      = 7;
}

class CResourceTreeItem
{
public:
  explicit CResourceTreeItem(EResourceTreeItemType type,
                             std::optional<EResourceType> resourceType = std::nullopt,
                             const QString& sLabel = QString(),
                             tspResource spResource = nullptr,
                             CResourceTreeItem* pParentItem = nullptr);
  ~CResourceTreeItem();

  void AppendChild(CResourceTreeItem* pChild);
  QString Label() { return m_sLabel; }
  tspResource Resource() { return m_spResource; }
  EResourceTreeItemType Type() { return m_type; }
  void SetLabel(const QString& sLabel) { m_sLabel = sLabel; }
  void SetResource(const tspResource& spResource) { m_spResource = spResource; }
  void SetType(const EResourceTreeItemType& type) { m_type = type; }
  void SetWarning(const QString& sWarning) { m_sWarning = sWarning; }

  CResourceTreeItem* Child(qint32 iRow);
  qint32 ChildIndex(CResourceTreeItem* pCompare);
  qint32 ChildCount() const;
  qint32 ColumnCount() const;
  QVariant Data(qint32 iColumn) const;
  QVariant HeaderData(qint32 iColumn) const;
  bool InsertChildren(qint32 iPosition, qint32 iCount, qint32 iColumns);
  bool InsertColumns(qint32 iPosition, qint32 iColumns);
  bool RemoveChildren(qint32 iPosition, qint32 iCount);
  bool RemoveColumns(qint32 iPosition, qint32 iColumns);
  CResourceTreeItem* Parent();
  qint32 Row() const;
  bool SetData(qint32 iColumn, const QVariant &value);

private:
  CResourceTreeItem*          m_pParentItem;
  QVector<CResourceTreeItem*> m_vpChildItems;
  EResourceTreeItemType       m_type;
  std::optional<EResourceType>m_resourceType;
  QString                     m_sLabel;
  QString                     m_sWarning;
  tspResource                 m_spResource;
};

#endif // RESOURCETREEITEM_H
