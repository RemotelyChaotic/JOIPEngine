#include "KinkTreeSortFilterProxyModel.h"
#include "KinkTreeItem.h"
#include "KinkTreeModel.h"

CKinkTreeSortFilterProxyModel::CKinkTreeSortFilterProxyModel(QObject* pParent) :
  QSortFilterProxyModel(pParent)
{
}

CKinkTreeSortFilterProxyModel::~CKinkTreeSortFilterProxyModel()
{
}

//----------------------------------------------------------------------------------------
//
void CKinkTreeSortFilterProxyModel::InitializeModel()
{
  CKinkTreeModel* pModel = dynamic_cast<CKinkTreeModel*>(sourceModel());
  if (nullptr != pModel)
  {
    pModel->InitializeModel();
  }
}

//----------------------------------------------------------------------------------------
//
void CKinkTreeSortFilterProxyModel::DeInitializeModel()
{
  CKinkTreeModel* pModel = dynamic_cast<CKinkTreeModel*>(sourceModel());
  if (nullptr != pModel)
  {
    pModel->DeInitializeModel();
  }
}

//----------------------------------------------------------------------------------------
//
bool CKinkTreeSortFilterProxyModel::filterAcceptsRow(int iSourceRow,
                                                     const QModelIndex& sourceParent) const
{
  CKinkTreeModel* pSourceModel = dynamic_cast<CKinkTreeModel*>(sourceModel());
  if (nullptr != pSourceModel)
  {
    if(!filterRegExp().isEmpty())
    {
        // get source-model index for current row
        QModelIndex sourceIndex = pSourceModel->index(iSourceRow, filterKeyColumn(), sourceParent);
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
          // check current index itself :
          QString key = sourceModel()->data(sourceIndex, filterRole()).toString();
          return key.contains(filterRegExp()) ;
        }
    }
  }

  return QSortFilterProxyModel::filterAcceptsRow(iSourceRow, sourceParent);
}

//----------------------------------------------------------------------------------------
//
bool CKinkTreeSortFilterProxyModel::lessThan(const QModelIndex& left,
                                             const QModelIndex& right) const
{
  return QSortFilterProxyModel::lessThan(left, right);
}
