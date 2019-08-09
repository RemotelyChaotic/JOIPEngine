#include "ResourceTreeItemSortFilterProxyModel.h"
#include "ResourceTreeItem.h"
#include "ResourceTreeItemModel.h"

CResourceTreeItemSortFilterProxyModel::CResourceTreeItemSortFilterProxyModel(QObject* pParent) :
  QSortFilterProxyModel(pParent)
{
}

CResourceTreeItemSortFilterProxyModel::~CResourceTreeItemSortFilterProxyModel()
{

}

//----------------------------------------------------------------------------------------
//
void CResourceTreeItemSortFilterProxyModel::InitializeModel(tspProject spProject)
{
  CResourceTreeItemModel* pModel = dynamic_cast<CResourceTreeItemModel*>(sourceModel());
  pModel->InitializeModel(spProject);
}

//----------------------------------------------------------------------------------------
//
void CResourceTreeItemSortFilterProxyModel::DeInitializeModel()
{
  CResourceTreeItemModel* pModel = dynamic_cast<CResourceTreeItemModel*>(sourceModel());
  pModel->DeInitializeModel();
}

//----------------------------------------------------------------------------------------
//
bool CResourceTreeItemSortFilterProxyModel::filterAcceptsRow(int iSourceRow,
                                                             const QModelIndex &sourceParent) const
{
  QModelIndex index = sourceModel()->index(iSourceRow, 0, sourceParent);
  return sourceModel()->data(index).toString().contains(filterRegExp());
}

//----------------------------------------------------------------------------------------
//
bool CResourceTreeItemSortFilterProxyModel::lessThan(const QModelIndex& left,
                                                     const QModelIndex& right) const
{
  QVariant leftData = sourceModel()->data(left, Qt::DisplayRole);
  QVariant rightData = sourceModel()->data(right, Qt::DisplayRole);

  switch(left.column())
  {
    case resource_item::c_iColumnName:
    {
      return leftData.toString() < rightData.toString();
    }
    case resource_item::c_iColumnType:
    {
      return leftData.toInt() < rightData.toInt();
    }
    case resource_item::c_iColumnPath:
    {
      return leftData.toUrl() < rightData.toUrl();
    }
  default:break;
  }

  return QSortFilterProxyModel::lessThan(left, right);
}
