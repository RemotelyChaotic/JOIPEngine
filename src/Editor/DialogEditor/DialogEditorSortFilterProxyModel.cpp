#include "DialogEditorSortFilterProxyModel.h"

CDialogEditorSortFilterProxyModel::CDialogEditorSortFilterProxyModel(QObject* pParent) :
    QSortFilterProxyModel(pParent)
{
  m_collator.setNumericMode(true);
}

CDialogEditorSortFilterProxyModel::~CDialogEditorSortFilterProxyModel()
{

}

//----------------------------------------------------------------------------------------
//
void CDialogEditorSortFilterProxyModel::setSourceModel(QAbstractItemModel* pSourceModel)
{
  beginResetModel();
  if (nullptr != sourceModel())
  {
    disconnect(sourceModel(), &QAbstractItemModel::rowsInserted,
               this, &CDialogEditorSortFilterProxyModel::SlotResourceAdded);
    disconnect(sourceModel(), &QAbstractItemModel::rowsRemoved,
               this, &CDialogEditorSortFilterProxyModel::SlotResourceRemoved);
  }

  QSortFilterProxyModel::setSourceModel(pSourceModel);

  if (nullptr != pSourceModel)
  {
    connect(pSourceModel, &QAbstractItemModel::rowsInserted,
            this, &CDialogEditorSortFilterProxyModel::SlotResourceAdded);
    connect(pSourceModel, &QAbstractItemModel::rowsRemoved,
            this, &CDialogEditorSortFilterProxyModel::SlotResourceRemoved);
  }
  endResetModel();
}

//----------------------------------------------------------------------------------------
//
bool CDialogEditorSortFilterProxyModel::filterAcceptsRow(int iSourceRow,
                                                         const QModelIndex &sourceParent) const
{
  return QSortFilterProxyModel::filterAcceptsRow(iSourceRow, sourceParent);
}

//----------------------------------------------------------------------------------------
//
bool CDialogEditorSortFilterProxyModel::lessThan(const QModelIndex& left,
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

//----------------------------------------------------------------------------------------
//
void CDialogEditorSortFilterProxyModel::SlotResourceAdded()
{
  invalidateFilter();
}

//----------------------------------------------------------------------------------------
//
void CDialogEditorSortFilterProxyModel::SlotResourceRemoved()
{
  invalidateFilter();
}
