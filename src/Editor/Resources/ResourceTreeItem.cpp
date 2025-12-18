#include "ResourceTreeItem.h"
#include "Application.h"
#include "Systems/DatabaseManager.h"

using namespace resource_item;

//----------------------------------------------------------------------------------------
//
CResourceTreeItem::CResourceTreeItem(EResourceTreeItemType type,
                                     std::optional<EResourceType> resourceType,
                                     const QString& sLabel,
                                     tspResource spResource,
                                     CResourceTreeItem* pParent) :
  m_pParentItem(pParent),
  m_type(type),
  m_resourceType(resourceType),
  m_sLabel(sLabel),
  m_spResource(spResource)
{}

CResourceTreeItem::~CResourceTreeItem()
{
  qDeleteAll(m_vpChildItems);
}

//----------------------------------------------------------------------------------------
//
void CResourceTreeItem::AddWarning(EWarningType type, const QString& sWarning)
{
  m_vsWarnings[type] = sWarning;
}

//----------------------------------------------------------------------------------------
//
void CResourceTreeItem::AppendChild(CResourceTreeItem* pItem)
{
  m_vpChildItems.append(pItem);
}

//----------------------------------------------------------------------------------------
//
bool CResourceTreeItem::ClearWarning(EWarningType type)
{
  if (auto it = m_vsWarnings.find(type); m_vsWarnings.end() != it)
  {
    m_vsWarnings.erase(it);
    return true;
  }
  return false;
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
qint32 CResourceTreeItem::ChildIndex(CResourceTreeItem* pCompare)
{
  for (qint32 i = 0; ChildCount() > i; ++i)
  {
    if (nullptr != pCompare &&
        pCompare->Data(c_iColumnName) == m_vpChildItems[i]->Data(c_iColumnName))
    {
      return i;
    }
  }
  return -1;
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
  if (iColumn < 0 || iColumn >= c_iNumItems)
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
        switch(iColumn)
        {
          case c_iColumnName:
            return QT_TR_NOOP(m_sLabel);
          case c_iColumnType:
            return m_resourceType.has_value() ? m_resourceType.value()._to_integral() : QVariant();
          default: return QVariant();
        }
      }
      case EResourceTreeItemType::eResource:
      {
        if (nullptr == m_spResource) { return QVariant(); }
        QReadLocker locker(&m_spResource->m_rwLock);
        switch(iColumn)
        {
          case c_iColumnName:
            return m_spResource->m_sName;
          case c_iColumnPath:
            return static_cast<QUrl>(m_spResource->m_sPath);
          case c_iColumnType:
            return m_spResource->m_type._to_integral();
          case c_iColumnToolTip:
            return m_spResource->m_sSource.isEmpty() ?
                  QVariant() : QString("Source: ") + m_spResource->m_sSource.toString();
          case c_iColumnLoadedID:
            return m_spResource->m_iLoadedId;
          case c_iColumnWarning:
            if (!m_vsWarnings.empty())
            {
              QStringList vsRet;
              for (const auto& [_, str] : m_vsWarnings)
              {
                vsRet << str;
              }
              return vsRet.join("\n");
            }
            return QString();
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
bool CResourceTreeItem::InsertChildren(qint32 iPosition, qint32 iCount, qint32 iColumns)
{
  Q_UNUSED(iColumns)
  if (iPosition < 0 || iPosition > m_vpChildItems.size())
  {
    return false;
  }

  for (qint32 iRow = 0; iRow < iCount; ++iRow)
  {
    CResourceTreeItem* pItem = new CResourceTreeItem(EResourceTreeItemType::eRoot,
                                                     std::nullopt,
                                                     QString(), nullptr, this);
    m_vpChildItems.insert(iPosition, pItem);
  }

  return true;
}

//----------------------------------------------------------------------------------------
//
bool CResourceTreeItem::InsertColumns(qint32 iPosition, qint32 iColumns)
{
  Q_UNUSED(iPosition)
  Q_UNUSED(iColumns)
  return true;
}

//----------------------------------------------------------------------------------------
//
bool CResourceTreeItem::RemoveChildren(qint32 iPosition, qint32 iCount)
{
  if (iPosition < 0 || iPosition + iCount > m_vpChildItems.size())
  {
    return false;
  }

  for (qint32 iRow = 0; iRow < iCount; ++iRow)
  {
    delete m_vpChildItems.takeAt(iPosition);
  }

  return true;
}

//----------------------------------------------------------------------------------------
//
bool CResourceTreeItem::RemoveColumns(qint32 iPosition, qint32 iColumns)
{
  Q_UNUSED(iPosition)
  Q_UNUSED(iColumns)
  return true;
}

//----------------------------------------------------------------------------------------
//
CResourceTreeItem* CResourceTreeItem::Parent()
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

//----------------------------------------------------------------------------------------
//
bool CResourceTreeItem::SetData(qint32 iColumn, const QVariant &value)
{
  if (iColumn < 0 || iColumn >= ColumnCount())
  {
    return false;
  }

  switch (m_type)
  {
    case EResourceTreeItemType::eRoot:    // fallthrough
    case EResourceTreeItemType::eCategory:// fallthrough
    case EResourceTreeItemType::eFolder:
    {
      if (iColumn == 0)
      {
        m_sLabel = value.toString();
      }
      else
      {
        return false;
      }
    } break;
    case EResourceTreeItemType::eResource:
    {
      auto spDbManager = CApplication::Instance()->System<CDatabaseManager>().lock();
      if (nullptr == m_spResource || nullptr == spDbManager) { return false; }
      QWriteLocker locker(&m_spResource->m_rwLock);
      switch(iColumn)
      {
        case c_iColumnName:
        {
          const QString sNewName = value.toString();
          if (!sNewName.isNull() && !sNewName.isEmpty())
          {
            const QString sOldName = m_spResource->m_sName;
            tspProject spProject = m_spResource->m_spParent;
            locker.unlock();
            spDbManager->RenameResource(spProject, sOldName, sNewName);
          }
          break;
        }
      default: break;
      }
    }
  }
  return true;
}

