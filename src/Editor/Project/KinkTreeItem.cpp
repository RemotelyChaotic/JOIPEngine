#include "KinkTreeItem.h"

CKinkTreeItem::CKinkTreeItem(QString sKinkCategory, tspKink spKink, CKinkTreeItem* pParentItem) :
  m_vpChildItems(),
  m_pParentItem(pParentItem),
  m_spKink(spKink),
  m_sKinkCategory(sKinkCategory)
{
}

CKinkTreeItem::~CKinkTreeItem()
{
  qDeleteAll(m_vpChildItems);
}

//----------------------------------------------------------------------------------------
//
void CKinkTreeItem::AppendChild(CKinkTreeItem* pChild)
{
  m_vpChildItems.append(pChild);
}

//----------------------------------------------------------------------------------------
//
CKinkTreeItem* CKinkTreeItem::Child(qint32 iRow)
{
  if (iRow < 0 || iRow >= m_vpChildItems.size())
  {
    return nullptr;
  }
  return m_vpChildItems.at(iRow);
}

//----------------------------------------------------------------------------------------
//
qint32 CKinkTreeItem::ChildCount() const
{
  return m_vpChildItems.count();
}

//----------------------------------------------------------------------------------------
//
QVariant CKinkTreeItem::Data(qint32 iRole) const
{
  if (nullptr == m_spKink)
  {
    switch (iRole)
    {
      case Qt::DisplayRole:
      {
        return m_sKinkCategory;
      }
      default: return QVariant();
    }
  }
  else
  {
    switch (iRole)
    {
      case Qt::DisplayRole:
      {
        QReadLocker locker(&m_spKink->m_rwLock);
        return m_spKink->m_sName;
      }
      case Qt::ToolTipRole:
      {
        QReadLocker locker(&m_spKink->m_rwLock);
        return m_spKink->m_sDescribtion;
      }
      default: return QVariant();
    }
  }
}

//----------------------------------------------------------------------------------------
//
qint32 CKinkTreeItem::Row() const
{
  if (m_pParentItem)
  {
    return m_pParentItem->m_vpChildItems.indexOf(const_cast<CKinkTreeItem*>(this));
  }
  return 0;
}

//----------------------------------------------------------------------------------------
//
CKinkTreeItem* CKinkTreeItem::Parent()
{
  return m_pParentItem;
}
