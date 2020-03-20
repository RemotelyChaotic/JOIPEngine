#ifndef RESOURCETREEITEMSORTFILTERPROXYMODEL_H
#define RESOURCETREEITEMSORTFILTERPROXYMODEL_H

#include "Systems/Project.h"
#include <QSortFilterProxyModel>

class CResourceTreeItemSortFilterProxyModel : public QSortFilterProxyModel
{
public:
  explicit CResourceTreeItemSortFilterProxyModel(QObject* pParent = nullptr);
  ~CResourceTreeItemSortFilterProxyModel() override;

  void InitializeModel(tspProject spProject);
  void DeInitializeModel();

protected:
    bool filterAcceptsRow(int iSourceRow, const QModelIndex& sourceParent) const override;
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;
};

#endif // RESOURCETREEITEMSORTFILTERPROXYMODEL_H
