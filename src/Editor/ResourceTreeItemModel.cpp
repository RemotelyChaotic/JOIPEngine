#include "ResourceTreeItemModel.h"
#include "ResourceTreeItem.h"

CResourceTreeItemModel::CResourceTreeItemModel(QObject* pParent) :
  QAbstractItemModel(pParent)
{
  m_pRootItem = new CResourceTreeItem(EResourceTreeItemType::eRoot, "Resources");

  m_categoryMap.insert({EResourceType::eImage,
    new CResourceTreeItem(EResourceTreeItemType::eCategory, "Images", nullptr, m_pRootItem)});
  m_categoryMap.insert({EResourceType::eMovie,
    new CResourceTreeItem(EResourceTreeItemType::eCategory, "Movies", nullptr, m_pRootItem)});
  m_categoryMap.insert({EResourceType::eSound,
    new CResourceTreeItem(EResourceTreeItemType::eCategory, "Sound", nullptr, m_pRootItem)});
  m_categoryMap.insert({EResourceType::eOther,
    new CResourceTreeItem(EResourceTreeItemType::eCategory, "Other", nullptr, m_pRootItem)});

  m_pRootItem->AppendChild(m_categoryMap[EResourceType::eImage]);
  m_pRootItem->AppendChild(m_categoryMap[EResourceType::eMovie]);
  m_pRootItem->AppendChild(m_categoryMap[EResourceType::eSound]);
  m_pRootItem->AppendChild(m_categoryMap[EResourceType::eOther]);
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

  m_categoryMap.insert({EResourceType::eImage,
    new CResourceTreeItem(EResourceTreeItemType::eCategory, "Images", nullptr, m_pRootItem)});
  m_categoryMap.insert({EResourceType::eMovie,
    new CResourceTreeItem(EResourceTreeItemType::eCategory, "Movies", nullptr, m_pRootItem)});
  m_categoryMap.insert({EResourceType::eSound,
    new CResourceTreeItem(EResourceTreeItemType::eCategory, "Sound", nullptr, m_pRootItem)});
  m_categoryMap.insert({EResourceType::eOther,
    new CResourceTreeItem(EResourceTreeItemType::eCategory, "Other", nullptr, m_pRootItem)});

  m_pRootItem->AppendChild(m_categoryMap[EResourceType::eImage]);
  m_pRootItem->AppendChild(m_categoryMap[EResourceType::eMovie]);
  m_pRootItem->AppendChild(m_categoryMap[EResourceType::eSound]);
  m_pRootItem->AppendChild(m_categoryMap[EResourceType::eOther]);
}

//----------------------------------------------------------------------------------------
//
QVariant CResourceTreeItemModel::data(const QModelIndex& index, int iRole) const
{
  if (!index.isValid()) { return QVariant(); }
  if (iRole != Qt::DisplayRole) { return QVariant(); }

  CResourceTreeItem* item = static_cast<CResourceTreeItem*>(index.internalPointer());
  return item->Data(index.column());
}

//----------------------------------------------------------------------------------------
//
Qt::ItemFlags CResourceTreeItemModel::flags(const QModelIndex& index) const
{
  if (!index.isValid()) { return Qt::NoItemFlags; }

  return QAbstractItemModel::flags(index);
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
  if (!hasIndex(iRow, iColumn, parent)) { return QModelIndex(); }

  CResourceTreeItem* pParentItem;
  if (!parent.isValid())
  {
    pParentItem = m_pRootItem;
  }
  else
  {
    pParentItem = static_cast<CResourceTreeItem*>(parent.internalPointer());
  }

  CResourceTreeItem* pChildItem = pParentItem->Child(iRow);
  if (pChildItem)
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

  CResourceTreeItem* pChildItem = static_cast<CResourceTreeItem*>(index.internalPointer());
  CResourceTreeItem* pParentItem = pChildItem->ParentItem();

  if (pParentItem == m_pRootItem) { return QModelIndex(); }
  return createIndex(pParentItem->Row(), 0, pParentItem);
}

//----------------------------------------------------------------------------------------
//
int CResourceTreeItemModel::rowCount(const QModelIndex& parent) const
{
  CResourceTreeItem* pParentItem;
  if (parent.column() > 0) { return 0; }

  if (!parent.isValid())
  {
    pParentItem = m_pRootItem;
  }
  else
  {
    pParentItem = static_cast<CResourceTreeItem*>(parent.internalPointer());
  }
  return pParentItem->ChildCount();
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
