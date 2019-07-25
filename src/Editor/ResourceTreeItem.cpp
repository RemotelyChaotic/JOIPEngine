#include "ResourceTreeItem.h"

namespace
{
  const qint32 c_iColumnName  = 0;
  const qint32 c_iColumnType  = 1;
  const qint32 c_iColumnPath  = 2;

  const qint32 c_iNumColumns  = 3;
}

//----------------------------------------------------------------------------------------
//
CResourceTreeItem::CResourceTreeItem(EResourceTreeItemType type,  const QString& sLabel,
  tspResource spResource, CResourceTreeItem* pParent) :
  m_pParentItem(pParent),
  m_type(type),
  m_sLabel(sLabel),
  m_spResource(spResource)
{}

CResourceTreeItem::~CResourceTreeItem()
{
  qDeleteAll(m_vpChildItems);
}

//----------------------------------------------------------------------------------------
//
void CResourceTreeItem::AppendChild(CResourceTreeItem* pItem)
{
  m_vpChildItems.append(pItem);
}

//----------------------------------------------------------------------------------------
//
CResourceTreeItem* CResourceTreeItem::Child(qint32 iRow)
{
  if (iRow < 0 || iRow >= m_vpChildItems.size())
  {
    return nullptr;
  }
  return m_vpChildItems.at(iRow);
}

//----------------------------------------------------------------------------------------
//
qint32 CResourceTreeItem::ChildCount() const
{
  return m_vpChildItems.count();
}

//----------------------------------------------------------------------------------------
//
qint32 CResourceTreeItem::ColumnCount() const
{
  return c_iNumColumns;
}

//----------------------------------------------------------------------------------------
//
QVariant CResourceTreeItem::Data(qint32 iColumn) const
{
  if (iColumn < 0 || iColumn >= c_iNumColumns)
  {
    return QVariant();
  }
  else
  {
    switch (m_type)
    {
      case EResourceTreeItemType::eRoot:    // fallthrough
      case EResourceTreeItemType::eCategory:// fallthrough
      case EResourceTreeItemType::eFolder:
      {
        if (iColumn == 0)
        {
          return QT_TR_NOOP(m_sLabel);
        }
        else
        {
          return QVariant();
        }
      }
      case EResourceTreeItemType::eResource:
      {
        QReadLocker locker(&m_spResource->m_rwLock);
        switch(iColumn)
        {
          case c_iColumnName:
            return m_spResource->m_sName;
          case c_iColumnPath:
            return m_spResource->m_sPath;
          case c_iColumnType:
            return m_spResource->m_type._to_integral();
        default: return QVariant();
        }
      }
      default: return QVariant();
    }
  }
}

//----------------------------------------------------------------------------------------
//
QVariant CResourceTreeItem::HeaderData(qint32 iColumn) const
{
  if (iColumn < 0 || iColumn >= c_iNumColumns)
  {
    return QVariant();
  }
  else
  {
    switch(iColumn)
    {
      case c_iColumnName:
        return "Name";
      case c_iColumnPath:
        return "Path";
      case c_iColumnType:
        return "Type";
    default: return QVariant();
    }
  }
}

//----------------------------------------------------------------------------------------
//
CResourceTreeItem* CResourceTreeItem::ParentItem()
{
  return m_pParentItem;
}

//----------------------------------------------------------------------------------------
//
int CResourceTreeItem::Row() const
{
  if (m_pParentItem)
  {
    return m_pParentItem->m_vpChildItems.indexOf(const_cast<CResourceTreeItem*>(this));
  }
  return 0;
}

