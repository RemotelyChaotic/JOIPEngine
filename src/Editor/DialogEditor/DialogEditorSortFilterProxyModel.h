#ifndef CDIALOGEDITORSORTFILTERPROXYMODEL_H
#define CDIALOGEDITORSORTFILTERPROXYMODEL_H

#include <QCollator>
#include <QSortFilterProxyModel>

class CDialogEditorSortFilterProxyModel : public QSortFilterProxyModel
{
public:
  CDialogEditorSortFilterProxyModel(QObject* pParent = nullptr);
  ~CDialogEditorSortFilterProxyModel() override;

  void setSourceModel(QAbstractItemModel* pSourceModel) override;

protected:
  bool filterAcceptsRow(int iSourceRow, const QModelIndex& sourceParent) const override;
  bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

private slots:
  void SlotResourceAdded();
  void SlotResourceRemoved();

private:
  QCollator                  m_collator;
};

#endif // CDIALOGEDITORSORTFILTERPROXYMODEL_H
