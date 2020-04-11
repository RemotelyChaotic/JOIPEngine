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
  m_pRootItem(nullptr)
{
  Initialize();
}
CKinkTreeModel::~CKinkTreeModel()
{
  DeInitializeModel();
}

//----------------------------------------------------------------------------------------
//
void CKinkTreeModel::Initialize()
{
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
QVariant CKinkTreeModel::data(const QModelIndex& index, int iRole) const
{
  if (!index.isValid()) { return QVariant(); }
  if (Qt::DisplayRole == iRole || Qt::ToolTipRole == iRole)
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
  else { return Qt::ItemIsSelectable | Qt::ItemIsEnabled; }
}

//----------------------------------------------------------------------------------------
//
QVariant CKinkTreeModel::headerData(int iSection, Qt::Orientation orientation,
                                    int iRole) const
{
  if (Qt::Horizontal == orientation && Qt::DisplayRole == iRole &&
      0 == iSection)
  {
    return tr("Kink");
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
CKinkTreeItem* CKinkTreeModel::GetItem(const QModelIndex& index) const
{
  if (index.isValid())
  {
    CKinkTreeItem* pItem = static_cast<CKinkTreeItem*>(index.internalPointer());
    if (nullptr != pItem) { return pItem; }
  }
  return m_pRootItem;
}
