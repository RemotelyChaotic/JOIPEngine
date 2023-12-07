#ifndef CPATTERNELEMENTLIST_H
#define CPATTERNELEMENTLIST_H

#include <QPointer>
#include <QStandardItemModel>
#include <QTreeView>

#include <functional>

class CSequenceEmentList : public QTreeView
{
  Q_OBJECT
public:
  explicit CSequenceEmentList(QWidget* pParent = nullptr);
  ~CSequenceEmentList() override;

  void Initialize();
  void Deinitalize();

  void ExpandAll();

private:
  void IterateItems(const QModelIndex& index,
                    const std::function<void(const QModelIndex&)>& fnToCall);

  QPointer<QStandardItemModel> m_pModel;
};

#endif // CPATTERNELEMENTLIST_H
