#include "KinkTreeModel.h"
#include "Application.h"
#include "KinkTreeItem.h"
#include "Systems/DatabaseManager.h"

namespace  {
  const qint32 c_iNumberCols = 1;
}

CKinkTreeModel::CKinkTreeModel(QObject* pParent) :
  QAbstractItemModel (pParent),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_pRootItem(nullptr),
  m_bReadOnly(false)
{
  if (auto spDbManager = m_wpDbManager.lock())
  {
    connect(spDbManager.get(), &CDatabaseManager::SignalReloadFinished,
            this, &CKinkTreeModel::SlotDbReloadFinished, Qt::QueuedConnection);
    InitializeModel();
  }
}
CKinkTreeModel::~CKinkTreeModel()
{
  DeInitializeModel();
}

//----------------------------------------------------------------------------------------
//
void CKinkTreeModel::InitializeModel()
{
  if (auto spDbManager = m_wpDbManager.lock())
  {
    if (!spDbManager->IsDbLoaded())
    {
      return;
    }
  }

  if (nullptr == m_pRootItem)
  {
    m_pRootItem = new CKinkTreeItem("", nullptr, nullptr);
    beginResetModel();
    if (auto spDbManager = m_wpDbManager.lock())
    {
      QStringList vsCategories = spDbManager->KinkCategories();
      for (const QString& sCategory : qAsConst(vsCategories))
      {
        CKinkTreeItem* pCategory = new CKinkTreeItem(sCategory, nullptr, m_pRootItem);
        m_pRootItem->AppendChild(pCategory);

        QStringList sKinks = spDbManager->FindKinks(sCategory);
        for (QString sKink : qAsConst(sKinks))
        {
          CKinkTreeItem* pItem = new CKinkTreeItem(sKink,
                                                   spDbManager->FindKink(sCategory, sKink),
                                                   pCategory);
          pCategory->AppendChild(pItem);
        }
      }
    }
    endResetModel();
  }
}

//----------------------------------------------------------------------------------------
//
void CKinkTreeModel::DeInitializeModel()
{
  if (nullptr != m_pRootItem)
  {
    beginResetModel();
    delete m_pRootItem;
    m_pRootItem = nullptr;
    endResetModel();
  }
}

//----------------------------------------------------------------------------------------
//
void CKinkTreeModel::ResetSelections()
{
  if (nullptr != m_pRootItem)
  {
    for (qint32 i = 0; m_pRootItem->ChildCount() > i; ++i)
    {
      for (qint32 j = 0; m_pRootItem->Child(i)->ChildCount() > j; ++j)
      {
        m_pRootItem->SetData(Qt::Unchecked, Qt::CheckStateRole);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CKinkTreeModel::ResetSelections(QStringList vsUnselectedKinks)
{
  if (nullptr != m_pRootItem)
  {
    for (qint32 i = 0; m_pRootItem->ChildCount() > i; ++i)
    {
      for (qint32 j = 0; m_pRootItem->Child(i)->ChildCount() > j; ++j)
      {
        CKinkTreeItem* pItem = m_pRootItem->Child(i)->Child(j);
        if (vsUnselectedKinks.contains(pItem->Data(Qt::DisplayRole).toString()))
        {
          pItem->SetData(Qt::Unchecked, Qt::CheckStateRole);
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CKinkTreeModel::SetSelections(QStringList vsSelectedKinks)
{
  if (nullptr != m_pRootItem)
  {
    for (qint32 i = 0; m_pRootItem->ChildCount() > i; ++i)
    {
      for (qint32 j = 0; m_pRootItem->Child(i)->ChildCount() > j; ++j)
      {
        CKinkTreeItem* pItem = m_pRootItem->Child(i)->Child(j);
        if (vsSelectedKinks.contains(pItem->Data(Qt::DisplayRole).toString()))
        {
          pItem->SetData(Qt::Checked, Qt::CheckStateRole);
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
std::vector<tspKink> CKinkTreeModel::SelectedItems()
{
  std::vector<tspKink> vspKinks;
  if (nullptr != m_pRootItem)
  {
    for (qint32 i = 0; m_pRootItem->ChildCount() > i; ++i)
    {
      for (qint32 j = 0; m_pRootItem->Child(i)->ChildCount() > j; ++j)
      {
        CKinkTreeItem* pItem = m_pRootItem->Child(i)->Child(j);
        if (pItem->IsChecked())
        {
          vspKinks.push_back(pItem->KinkData());
        }
      }
    }
  }
  return vspKinks;
}

//----------------------------------------------------------------------------------------
//
QVariant CKinkTreeModel::data(const QModelIndex& index, int iRole) const
{
  if (!index.isValid()) { return QVariant(); }
  if (Qt::DisplayRole == iRole || Qt::ToolTipRole == iRole || Qt::CheckStateRole == iRole)
  {
    CKinkTreeItem* item = static_cast<CKinkTreeItem*>(index.internalPointer());
    return item->Data(iRole);
  }
  else
  {
    return QVariant();
  }
}

//----------------------------------------------------------------------------------------
//
Qt::ItemFlags CKinkTreeModel::flags(const QModelIndex& index) const
{
  if (!index.isValid()) { return Qt::NoItemFlags; }
  else
  {
    CKinkTreeItem* pChildItem = GetItem(index);
    CKinkTreeItem* pParentItem = nullptr != pChildItem ? pChildItem->Parent() : nullptr;
    if (pParentItem == m_pRootItem || nullptr == pParentItem) { return Qt::ItemIsSelectable | Qt::ItemIsEnabled; }
    else { return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable; }
  }
}

//----------------------------------------------------------------------------------------
//
QVariant CKinkTreeModel::headerData(int iSection, Qt::Orientation orientation,
                                    int iRole) const
{
  if (Qt::Horizontal == orientation && Qt::DisplayRole == iRole &&
      0 == iSection)
  {
    return tr("Fetish / Kink");
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
QModelIndex CKinkTreeModel::index(int iRow, int iColumn,
                                  const QModelIndex& parent) const
{
  // hasIndex checks if the values are in the valid ranges by using
  // rowCount and columnCount
  if (!hasIndex(iRow, iColumn, parent)) { return QModelIndex(); }

  CKinkTreeItem* pParentItem = GetItem(parent);
  CKinkTreeItem* pChildItem = pParentItem->Child(iRow);
  if (nullptr != pChildItem)
  {
      return createIndex(iRow, iColumn, pChildItem);
  }
  return QModelIndex();
}

//----------------------------------------------------------------------------------------
//
QModelIndex CKinkTreeModel::parent(const QModelIndex& index) const
{
  if (!index.isValid()) { return QModelIndex(); }

  CKinkTreeItem* pChildItem = GetItem(index);
  CKinkTreeItem* pParentItem = nullptr != pChildItem ? pChildItem->Parent() : nullptr;

  if (pParentItem == m_pRootItem || nullptr == pParentItem) { return QModelIndex(); }
  return createIndex(pParentItem->Row(), 0, pParentItem);
}

//----------------------------------------------------------------------------------------
//
int CKinkTreeModel::rowCount(const QModelIndex& parent) const
{
  const CKinkTreeItem* pParentItem = GetItem(parent);
  return nullptr != pParentItem ? pParentItem->ChildCount() : 0;
}

//----------------------------------------------------------------------------------------
//
int CKinkTreeModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  return c_iNumberCols;
}

//----------------------------------------------------------------------------------------
//
bool CKinkTreeModel::setData(const QModelIndex& index, const QVariant& value,
                             qint32 iRole)
{
  if (!index.isValid()) { return false; }

  CKinkTreeItem* pChildItem = GetItem(index);
  CKinkTreeItem* pParentItem = nullptr != pChildItem ? pChildItem->Parent() : nullptr;
  if (pParentItem == m_pRootItem || nullptr == pParentItem) { return false; }

  if (Qt::CheckStateRole == iRole)
  {
    if (m_bReadOnly) { return true; }

    bool bChanged = pChildItem->SetData(value, iRole);
    if (bChanged)
    {
      emit dataChanged(index, index, {Qt::CheckStateRole});
      emit SignalCheckedItem(index, value.toInt() == Qt::Checked);
    }
    return bChanged;
  }
  else
  {
    return false;
  }
}

//----------------------------------------------------------------------------------------
//
void CKinkTreeModel::SlotDbReloadFinished()
{
  DeInitializeModel();
  InitializeModel();
}

//----------------------------------------------------------------------------------------
//
CKinkTreeItem* CKinkTreeModel::GetItem(const QModelIndex& index) const
{
  if (index.isValid())
  {
    CKinkTreeItem* pItem = static_cast<CKinkTreeItem*>(index.internalPointer());
    if (nullptr != pItem) { return pItem; }
  }
  return m_pRootItem;
}
