#ifndef KINKTREESORTFILTERPROXYMODEL_H
#define KINKTREESORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class CKinkTreeSortFilterProxyModel : public QSortFilterProxyModel
{
public:
  explicit CKinkTreeSortFilterProxyModel(QObject* pParent = nullptr);
  ~CKinkTreeSortFilterProxyModel() override;

  void InitializeModel();
  void DeInitializeModel();

protected:
    bool filterAcceptsRow(int iSourceRow, const QModelIndex& sourceParent) const override;
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;
};

#endif // KINKTREESORTFILTERPROXYMODEL_H
