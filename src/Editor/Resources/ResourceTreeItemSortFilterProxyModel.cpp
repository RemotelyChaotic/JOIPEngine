#include "ResourceTreeItemSortFilterProxyModel.h"
#include "ResourceTreeItem.h"
#include "ResourceTreeItemModel.h"

CResourceTreeItemSortFilterProxyModel::CResourceTreeItemSortFilterProxyModel(QObject* pParent) :
  QSortFilterProxyModel(pParent)
{
  m_collator.setNumericMode(true);
}

CResourceTreeItemSortFilterProxyModel::~CResourceTreeItemSortFilterProxyModel()
{

}

//----------------------------------------------------------------------------------------
//
void CResourceTreeItemSortFilterProxyModel::InitializeModel(tspProject spProject)
{
  CResourceTreeItemModel* pModel = dynamic_cast<CResourceTreeItemModel*>(sourceModel());
  if (nullptr != pModel)
  {
    pModel->InitializeModel(spProject);
  }
}

//----------------------------------------------------------------------------------------
//
void CResourceTreeItemSortFilterProxyModel::DeInitializeModel()
{
  CResourceTreeItemModel* pModel = dynamic_cast<CResourceTreeItemModel*>(sourceModel());
  if (nullptr != pModel)
  {
    pModel->DeInitializeModel();
  }
}

//----------------------------------------------------------------------------------------
//
void CResourceTreeItemSortFilterProxyModel::FilterForTypes(
    const std::vector<EResourceType>& vEnabledTypes)
{
  if (m_vEnabledTypes != vEnabledTypes)
  {
    m_vEnabledTypes = vEnabledTypes;
    invalidateFilter();
  }
}

//----------------------------------------------------------------------------------------
//
void CResourceTreeItemSortFilterProxyModel::setSourceModel(QAbstractItemModel* pSourceModel)
{
  if (nullptr != sourceModel())
  {
    disconnect(sourceModel(), &CResourceTreeItemModel::rowsInserted,
               this, &CResourceTreeItemSortFilterProxyModel::SlotResourceAdded);
    disconnect(sourceModel(), &CResourceTreeItemModel::rowsRemoved,
               this, &CResourceTreeItemSortFilterProxyModel::SlotResourceRemoved);
  }

  QSortFilterProxyModel::setSourceModel(pSourceModel);

  if (nullptr != pSourceModel)
  {
    connect(pSourceModel, &CResourceTreeItemModel::rowsInserted,
            this, &CResourceTreeItemSortFilterProxyModel::SlotResourceAdded);
    connect(pSourceModel, &CResourceTreeItemModel::rowsRemoved,
            this, &CResourceTreeItemSortFilterProxyModel::SlotResourceRemoved);
  }
}

//----------------------------------------------------------------------------------------
//
bool CResourceTreeItemSortFilterProxyModel::filterAcceptsRow(int iSourceRow,
                                                             const QModelIndex &sourceParent) const
{
  CResourceTreeItemModel* pSourceModel = dynamic_cast<CResourceTreeItemModel*>(sourceModel());
  if (nullptr != pSourceModel)
  {
    // get source-model index for current row
    QModelIndex sourceIndex = pSourceModel->index(iSourceRow, 0, sourceParent);
    if(sourceIndex.isValid())
    {
      // check current index itself :
      QString key = pSourceModel->data(sourceIndex, Qt::DisplayRole, resource_item::c_iColumnName).toString();
      QVariant varType = pSourceModel->data(sourceIndex, Qt::DisplayRole, resource_item::c_iColumnType).toInt();
      bool bMatchesTsypeSelector = m_vEnabledTypes.empty() || !varType.isValid();
      if (!bMatchesTsypeSelector)
      {
        auto it =
            std::find(m_vEnabledTypes.begin(), m_vEnabledTypes.end(),
                      EResourceType::_from_integral(varType.toInt()));
        bMatchesTsypeSelector = m_vEnabledTypes.end() != it;
      }

      if(!filterRegExp().isEmpty())
      {
        if(sourceIndex.isValid())
        {
          // if any of children matches the filter, then current index matches the filter as well
          qint32 iNb = pSourceModel->rowCount(sourceIndex) ;
          for(qint32 i = 0; i < iNb; ++i)
          {
            if(filterAcceptsRow(i, sourceIndex))
            {
              return true;
            }
          }

          return bMatchesTsypeSelector && key.contains(filterRegExp());
        }
      }
      else
      {
        qint32 iNb = pSourceModel->rowCount(sourceIndex) ;
        for(qint32 i = 0; i < iNb; ++i)
        {
          if(filterAcceptsRow(i, sourceIndex))
          {
            return true;
          }
        }

        return bMatchesTsypeSelector;
      }
    }
  }

  return QSortFilterProxyModel::filterAcceptsRow(iSourceRow, sourceParent);
}

//----------------------------------------------------------------------------------------
//
bool CResourceTreeItemSortFilterProxyModel::lessThan(const QModelIndex& left,
                                                     const QModelIndex& right) const
{
  QVariant leftData = sourceModel()->data(left, Qt::DisplayRole);
  QVariant rightData = sourceModel()->data(right, Qt::DisplayRole);

  qint32 optTypeLeft = sourceModel()->data(left, CResourceTreeItemModel::eItemTypeRole).toInt();
  qint32 optTypeRight = sourceModel()->data(right, CResourceTreeItemModel::eItemTypeRole).toInt();

  bool bLeftHasChildren = optTypeLeft != EResourceTreeItemType::eResource;
  bool bRightHasChildren = optTypeRight != EResourceTreeItemType::eResource;

  if (bLeftHasChildren && !bRightHasChildren)
  {
    return true;
  }
  else if (!bLeftHasChildren && bRightHasChildren)
  {
    return false;
  }

  switch(left.column())
  {
    case resource_item::c_iColumnName:
    {
      return m_collator.compare(leftData.toString(), rightData.toString()) < 0;
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

//----------------------------------------------------------------------------------------
//
void CResourceTreeItemSortFilterProxyModel::SlotResourceAdded()
{
  invalidateFilter();
}

//----------------------------------------------------------------------------------------
//
void CResourceTreeItemSortFilterProxyModel::SlotResourceRemoved()
{
  invalidateFilter();
}
