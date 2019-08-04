#include "ResourceTreeItemModel.h"
#include "Application.h"
#include "ResourceTreeItem.h"
#include "Backend/DatabaseManager.h"

using namespace resource_item;

CResourceTreeItemModel::CResourceTreeItemModel(QObject* pParent) :
  QAbstractItemModel(pParent),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
{
  m_pRootItem = new CResourceTreeItem(EResourceTreeItemType::eRoot, "Resources");

  for (size_t i = 0; i < EResourceType::_size(); i++)
  {
    CResourceTreeItem* pItem =
        new CResourceTreeItem(EResourceTreeItemType::eCategory,
                            QString(EResourceType::_from_index(i)._to_string()).remove(0, 1),
                            nullptr, m_pRootItem);
    m_categoryMap.insert({EResourceType::_from_index(i), pItem});
    m_pRootItem->AppendChild(pItem);
  }

  auto spDbManager = m_wpDbManager.lock();
  connect(spDbManager.get(), &CDatabaseManager::SignalResourceAdded,
          this, &CResourceTreeItemModel::SlotResourceAdded, Qt::QueuedConnection);
  connect(spDbManager.get(), &CDatabaseManager::SignalResourceRemoved,
          this, &CResourceTreeItemModel::SlotResourceRemoved, Qt::QueuedConnection);
}

CResourceTreeItemModel::~CResourceTreeItemModel()
{
  m_categoryMap.clear();
  delete m_pRootItem;
}

//----------------------------------------------------------------------------------------
//
void CResourceTreeItemModel::InitializeModel(tspProject spProject)
{
  m_spProject = spProject;
  QReadLocker locker(&m_spProject->m_rwLock);
  for (auto it = m_spProject->m_spResourcesMap.begin(); m_spProject->m_spResourcesMap.end() != it; ++it)
  {
    QReadLocker locker(&it->second->m_rwLock);
    auto categoryIt = m_categoryMap.find(it->second->m_type);
    if (m_categoryMap.end() != categoryIt)
    {
      categoryIt->second->AppendChild(
            new CResourceTreeItem(EResourceTreeItemType::eResource, QString(), it->second, categoryIt->second));
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CResourceTreeItemModel::DeInitializeModel()
{
  m_spProject = nullptr;
  m_categoryMap.clear();
  delete m_pRootItem;

  m_pRootItem = new CResourceTreeItem(EResourceTreeItemType::eRoot, "Resources");

  for (size_t i = 0; i < EResourceType::_size(); i++)
  {
    CResourceTreeItem* pItem =
        new CResourceTreeItem(EResourceTreeItemType::eCategory,
                            QString(EResourceType::_from_index(i)._to_string()).remove(0, 1),
                            nullptr, m_pRootItem);
    m_categoryMap.insert({EResourceType::_from_index(i), pItem});
    m_pRootItem->AppendChild(pItem);
  }
}

//----------------------------------------------------------------------------------------
//
QVariant CResourceTreeItemModel::data(const QModelIndex& index, int iRole, int iColumnOverride)
{
  if (!index.isValid()) { return QVariant(); }
  if (Qt::DisplayRole == iRole)
  {
    CResourceTreeItem* item = static_cast<CResourceTreeItem*>(index.internalPointer());
    return item->Data(iColumnOverride);
  }
  // used for id to search
  else if (Qt::UserRole == iRole)
  {
    CResourceTreeItem* item = static_cast<CResourceTreeItem*>(index.internalPointer());
    return QString(item->Type()._to_string()) + ";" + item->Data(c_iColumnName).toString();
  }
  else
  {
    return QVariant();
  }
}

//----------------------------------------------------------------------------------------
//
QVariant CResourceTreeItemModel::data(const QModelIndex& index, int iRole) const
{
  if (!index.isValid()) { return QVariant(); }
  if (Qt::DisplayRole == iRole)
  {
    CResourceTreeItem* item = static_cast<CResourceTreeItem*>(index.internalPointer());
    return item->Data(index.column());
  }
  // used for id to search
  else if (Qt::UserRole == iRole)
  {
    CResourceTreeItem* item = static_cast<CResourceTreeItem*>(index.internalPointer());
    return QString(item->Type()._to_string()) + ";" + item->Data(c_iColumnName).toString();
  }
  else
  {
    return QVariant();
  }
}

//----------------------------------------------------------------------------------------
//
Qt::ItemFlags CResourceTreeItemModel::flags(const QModelIndex& index) const
{
  if (!index.isValid()) { return Qt::NoItemFlags; }

  CResourceTreeItem* pItem = GetItem(index);
  if (nullptr != pItem)
  {
    if (pItem->Parent() != m_pRootItem && pItem->Parent() != nullptr)
    {
      switch (index.column())
      {
        case c_iColumnName: return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
        default: return QAbstractItemModel::flags(index);
      }
    }
    else
    {
      return QAbstractItemModel::flags(index);
    }
  }
  else
  {
    return QAbstractItemModel::flags(index);
  }
}

//----------------------------------------------------------------------------------------
//
QVariant CResourceTreeItemModel::headerData(
    int iSection, Qt::Orientation orientation, int iRole) const
{
  if (Qt::Horizontal == orientation && Qt::DisplayRole == iRole)
  {
    return m_pRootItem->HeaderData(iSection);
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
QModelIndex CResourceTreeItemModel::index(
    int iRow, int iColumn, const QModelIndex& parent) const
{
  // hasIndex checks if the values are in the valid ranges by using
  // rowCount and columnCount
  if (!hasIndex(iRow, iColumn, parent)) { return QModelIndex(); }

  CResourceTreeItem* pParentItem = GetItem(parent);
  CResourceTreeItem* pChildItem = pParentItem->Child(iRow);
  if (nullptr != pChildItem)
  {
      return createIndex(iRow, iColumn, pChildItem);
  }
  return QModelIndex();
}

//----------------------------------------------------------------------------------------
//
QModelIndex CResourceTreeItemModel::parent(const QModelIndex& index) const
{
  if (!index.isValid()) { return QModelIndex(); }

  CResourceTreeItem* pChildItem = GetItem(index);
  CResourceTreeItem* pParentItem = nullptr != pChildItem ? pChildItem->Parent() : nullptr;;

  if (pParentItem == m_pRootItem || nullptr == pParentItem) { return QModelIndex(); }
  return createIndex(pParentItem->Row(), 0, pParentItem);
}

//----------------------------------------------------------------------------------------
//
int CResourceTreeItemModel::rowCount(const QModelIndex& parent) const
{
  const CResourceTreeItem* pParentItem = GetItem(parent);

  return nullptr != pParentItem ? pParentItem->ChildCount() : 0;
}

//----------------------------------------------------------------------------------------
//
int CResourceTreeItemModel::columnCount(const QModelIndex& parent) const
{
  if (parent.isValid())
  {
    return static_cast<CResourceTreeItem*>(parent.internalPointer())->ColumnCount();
  }
  return m_pRootItem->ColumnCount();
}

//----------------------------------------------------------------------------------------
//
bool CResourceTreeItemModel::setData(const QModelIndex& index, const QVariant& value,
             qint32 iRole)
{
  if (iRole != Qt::EditRole) { return false; }

  CResourceTreeItem* pItem = GetItem(index);
  bool bResult = pItem->SetData(index.column(), value);

  if (bResult)
  {
    emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
  }

  return bResult;
}

//----------------------------------------------------------------------------------------
//
bool CResourceTreeItemModel::setHeaderData(qint32 iSection, Qt::Orientation orientation,
                   const QVariant& value, qint32 iRole)
{
  if (iRole != Qt::EditRole || orientation != Qt::Horizontal) { return false; }

  const bool bResult = m_pRootItem->SetData(iSection, value);

  if (bResult) { emit headerDataChanged(orientation, iSection, iSection); }
  return bResult;
}

//----------------------------------------------------------------------------------------
//
bool CResourceTreeItemModel::insertColumns(qint32 iPosition, qint32 iColumns,
                   const QModelIndex& parent)
{
  beginInsertColumns(parent, iPosition, iPosition + iColumns - 1);
  const bool success = m_pRootItem->InsertColumns(iPosition, iColumns);
  endInsertColumns();

  return success;
}

//----------------------------------------------------------------------------------------
//
bool CResourceTreeItemModel::removeColumns(qint32 iPosition, qint32 iColumns,
                   const QModelIndex& parent)
{
  beginRemoveColumns(parent, iPosition, iPosition + iColumns - 1);
  const bool success = m_pRootItem->RemoveColumns(iPosition, iColumns);
  endRemoveColumns();

  if (m_pRootItem->ColumnCount() == 0) { removeRows(0, rowCount()); }
  return success;
}

//----------------------------------------------------------------------------------------
//
bool CResourceTreeItemModel::insertRows(qint32 iPosition, qint32 iRows,
                const QModelIndex& parent)
{
  CResourceTreeItem* pParentItem = GetItem(parent);
  if (!pParentItem) { return false; }

  beginInsertRows(parent, iPosition, iPosition + iRows - 1);
  const bool success = pParentItem->InsertChildren(
        iPosition, iRows, m_pRootItem->ColumnCount());
  endInsertRows();

  return success;
}

//----------------------------------------------------------------------------------------
//
bool CResourceTreeItemModel::removeRows(qint32 iPosition, qint32 iRows,
                const QModelIndex& parent)
{
  CResourceTreeItem* pParentItem = GetItem(parent);
  if (nullptr == pParentItem) { return false; }

  beginRemoveRows(parent, iPosition, iPosition + iRows - 1);
  const bool success = pParentItem->RemoveChildren(iPosition, iRows);
  endRemoveRows();

  return success;
}

//----------------------------------------------------------------------------------------
//
bool CResourceTreeItemModel::IsResourceType(const QModelIndex& index)
{
  if (!index.isValid()) { return false; }

  CResourceTreeItem* pItem = GetItem(index);
  return pItem->Type()._to_integral() == EResourceTreeItemType::eResource;
}

//----------------------------------------------------------------------------------------
//
void CResourceTreeItemModel::SlotResourceAdded(qint32 iProjId, const QString& sName)
{
  if (nullptr!= m_spProject)
  {
    m_spProject->m_rwLock.lockForRead();
    qint32 iThisId = m_spProject->m_iId;
    m_spProject->m_rwLock.unlock();

    if (iProjId == iThisId)
    {
      auto spDbManager = m_wpDbManager.lock();
      tspResource spResource = spDbManager->FindResource(m_spProject, sName);
      spResource->m_rwLock.lockForRead();
      EResourceType type = spResource->m_type;
      spResource->m_rwLock.unlock();

      auto itCategoryItem = m_categoryMap.find(type);
      QModelIndex parent =
          createIndex(static_cast<qint32>(type._to_index()), 0, m_categoryMap[type]);
      qint32 iPosition = m_categoryMap[type]->ChildCount();

      // insert item
      beginInsertRows(parent, iPosition, iPosition);
      const bool bSuccess = m_categoryMap[type]->InsertChildren(
            iPosition, 1, m_pRootItem->ColumnCount());
      if (bSuccess)
      {
        CResourceTreeItem* pNewItem = m_categoryMap[type]->Child(m_categoryMap[type]->ChildCount() - 1);
        pNewItem->SetType(EResourceTreeItemType::eResource);
        pNewItem->SetResource(spResource);
      }
      endInsertRows();
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CResourceTreeItemModel::SlotResourceRemoved(qint32 iProjId, const QString& sName)
{
  if (nullptr!= m_spProject)
  {
    m_spProject->m_rwLock.lockForRead();
    qint32 iThisId = m_spProject->m_iId;
    m_spProject->m_rwLock.unlock();

    if (iProjId == iThisId)
    {

      QModelIndexList indices =
        QAbstractItemModel::match(createIndex(0, 0, m_pRootItem), Qt::UserRole,
                                  QString(EResourceTreeItemType((EResourceTreeItemType::eResource))._to_string()) +
                                    ";" + sName, 1,
                                  Qt::MatchStartsWith | Qt::MatchWrap | Qt::MatchRecursive);
      if (0 < indices.size())
      {
        CResourceTreeItem* pItem = GetItem(indices[0]);
        CResourceTreeItem* pParent = pItem->Parent();
        qint32 iPosition = pParent->ChildIndex(pItem);

        // remove item
        beginRemoveRows(indices[0], iPosition, iPosition);
        const bool bSuccess = pParent->RemoveChildren(iPosition, 1);
        Q_UNUSED(bSuccess);
        endRemoveRows();
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
CResourceTreeItem* CResourceTreeItemModel::CResourceTreeItemModel::GetItem(const QModelIndex& index) const
{
  if (index.isValid())
  {
    CResourceTreeItem* pItem = static_cast<CResourceTreeItem*>(index.internalPointer());
    if (nullptr != pItem) { return pItem; }
  }
  return m_pRootItem;
}
