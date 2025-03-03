#include "DialogueEditorSortFilterProxyModel.h"

CDialogueEditorSortFilterProxyModel::CDialogueEditorSortFilterProxyModel(QObject* pParent) :
    QSortFilterProxyModel(pParent)
{
  m_collator.setNumericMode(true);
}

CDialogueEditorSortFilterProxyModel::~CDialogueEditorSortFilterProxyModel()
{

}

//----------------------------------------------------------------------------------------
//
void CDialogueEditorSortFilterProxyModel::setSourceModel(QAbstractItemModel* pSourceModel)
{
  beginResetModel();
  if (nullptr != sourceModel())
  {
    disconnect(sourceModel(), &QAbstractItemModel::rowsInserted,
               this, &CDialogueEditorSortFilterProxyModel::SlotResourceAdded);
    disconnect(sourceModel(), &QAbstractItemModel::rowsRemoved,
               this, &CDialogueEditorSortFilterProxyModel::SlotResourceRemoved);
  }

  QSortFilterProxyModel::setSourceModel(pSourceModel);

  if (nullptr != pSourceModel)
  {
    connect(pSourceModel, &QAbstractItemModel::rowsInserted,
            this, &CDialogueEditorSortFilterProxyModel::SlotResourceAdded);
    connect(pSourceModel, &QAbstractItemModel::rowsRemoved,
            this, &CDialogueEditorSortFilterProxyModel::SlotResourceRemoved);
  }
  endResetModel();
}

//----------------------------------------------------------------------------------------
//
bool CDialogueEditorSortFilterProxyModel::filterAcceptsRow(int iSourceRow,
                                                         const QModelIndex &sourceParent) const
{
  return QSortFilterProxyModel::filterAcceptsRow(iSourceRow, sourceParent);
}

//----------------------------------------------------------------------------------------
//
bool CDialogueEditorSortFilterProxyModel::lessThan(const QModelIndex& left,
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
void CDialogueEditorSortFilterProxyModel::SlotResourceAdded()
{
  invalidateFilter();
}

//----------------------------------------------------------------------------------------
//
void CDialogueEditorSortFilterProxyModel::SlotResourceRemoved()
{
  invalidateFilter();
}
