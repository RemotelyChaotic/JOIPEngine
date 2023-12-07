#include "SqeuenceElementListModel.h"

CSqeuenceElementListModel::CSqeuenceElementListModel(QObject* pParent) :
  QStandardItemModel(pParent)
{

}
CSqeuenceElementListModel::~CSqeuenceElementListModel() = default;

//----------------------------------------------------------------------------------------
//
CSqeuenceElementSortFilterProxyModel::CSqeuenceElementSortFilterProxyModel(QObject* pParent) :
  QSortFilterProxyModel(pParent)
{
  m_collator.setNumericMode(true);
}

CSqeuenceElementSortFilterProxyModel::~CSqeuenceElementSortFilterProxyModel()
{

}

//----------------------------------------------------------------------------------------
//
void CSqeuenceElementSortFilterProxyModel::setSourceModel(QAbstractItemModel* pSourceModel)
{
  QSortFilterProxyModel::setSourceModel(pSourceModel);
}

//----------------------------------------------------------------------------------------
//
bool CSqeuenceElementSortFilterProxyModel::filterAcceptsRow(int iSourceRow,
                                                            const QModelIndex &sourceParent) const
{
  CSqeuenceElementListModel* pSourceModel = dynamic_cast<CSqeuenceElementListModel*>(sourceModel());
  if (nullptr != pSourceModel)
  {
    if(!filterRegExp().isEmpty())
    {
      // get source-model index for current row
      QModelIndex sourceIndex = pSourceModel->index(iSourceRow, 0, sourceParent);
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
        QString key = pSourceModel->data(sourceIndex, Qt::DisplayRole).toString();
        //QVariant varType = pSourceModel->data(sourceIndex, Qt::DisplayRole).toInt();
        return /*bMatchesTsypeSelector && */key.contains(filterRegExp());
      }
    }
  }

  return QSortFilterProxyModel::filterAcceptsRow(iSourceRow, sourceParent);
}

//----------------------------------------------------------------------------------------
//
bool CSqeuenceElementSortFilterProxyModel::lessThan(const QModelIndex& left,
                                                    const QModelIndex& right) const
{
  QVariant leftData = sourceModel()->data(left, Qt::DisplayRole);
  QVariant rightData = sourceModel()->data(right, Qt::DisplayRole);

  bool bLeftHasChildren = sourceModel()->hasChildren(left);
  bool bRightHasChildren = sourceModel()->hasChildren(right);

  if (bLeftHasChildren && !bRightHasChildren)
  {
    return true;
  }
  else if (!bLeftHasChildren && bRightHasChildren)
  {
    return false;
  }

  return m_collator.compare(leftData.toString(), rightData.toString()) < 0;
}
