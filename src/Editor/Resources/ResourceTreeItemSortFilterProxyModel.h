#ifndef RESOURCETREEITEMSORTFILTERPROXYMODEL_H
#define RESOURCETREEITEMSORTFILTERPROXYMODEL_H

#include "Systems/Project.h"
#include <QSortFilterProxyModel>
#include <vector>

class CResourceTreeItemSortFilterProxyModel : public QSortFilterProxyModel
{
public:
  explicit CResourceTreeItemSortFilterProxyModel(QObject* pParent = nullptr);
  ~CResourceTreeItemSortFilterProxyModel() override;

  void InitializeModel(tspProject spProject);
  void DeInitializeModel();

  void FilterForTypes(const std::vector<EResourceType>& vEnabledTypes);

  void setSourceModel(QAbstractItemModel* pSourceModel) override;

protected:
  bool filterAcceptsRow(int iSourceRow, const QModelIndex& sourceParent) const override;
  bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

private:
  void SlotResourceAdded();
  void SlotResourceRemoved();

  std::vector<EResourceType> m_vEnabledTypes;
};

#endif // RESOURCETREEITEMSORTFILTERPROXYMODEL_H
