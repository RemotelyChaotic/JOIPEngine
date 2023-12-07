#ifndef CSQEUENCEELEMENTLISTMODEL_H
#define CSQEUENCEELEMENTLISTMODEL_H

#include <QCollator>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>

class CSqeuenceElementListModel : public QStandardItemModel
{
public:
  explicit CSqeuenceElementListModel(QObject* pParent = nullptr);
  ~CSqeuenceElementListModel();
};

//----------------------------------------------------------------------------------------
//
class CSqeuenceElementSortFilterProxyModel : public QSortFilterProxyModel
{
public:
  explicit CSqeuenceElementSortFilterProxyModel(QObject* pParent = nullptr);
  ~CSqeuenceElementSortFilterProxyModel() override;

  void setSourceModel(QAbstractItemModel* pSourceModel) override;

protected:
  bool filterAcceptsRow(int iSourceRow, const QModelIndex& sourceParent) const override;
  bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

private:
  QCollator                  m_collator;
};

#endif // CSQEUENCEELEMENTLISTMODEL_H
