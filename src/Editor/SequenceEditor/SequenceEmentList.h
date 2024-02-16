#ifndef CPATTERNELEMENTLIST_H
#define CPATTERNELEMENTLIST_H

#include <QPointer>
#include <QStandardItemModel>
#include <QTreeView>

#include <functional>

class CSqeuenceElementSortFilterProxyModel;

class CSequenceEmentList : public QTreeView
{
  Q_OBJECT
public:
  explicit CSequenceEmentList(QWidget* pParent = nullptr);
  ~CSequenceEmentList() override;

  void Initialize();
  void Deinitalize();

  void ExpandAll();
  void SetAllowedCategories(const QStringList& vsCategories);
  void SetFilter(const QString& sFilter);

signals:
  void SignalSelectedItem(const QString& sId);

protected slots:
  void SlotSelectionChanged(const QModelIndex& current, const QModelIndex& previous);

private:
  void IterateItems(const QModelIndex& index,
                    const std::function<void(const QModelIndex&)>& fnToCall);

  QPointer<QStandardItemModel>                   m_pModel;
  QPointer<CSqeuenceElementSortFilterProxyModel> m_pSortFilter;
  QStringList                                    m_vsCategories;
};

#endif // CPATTERNELEMENTLIST_H
