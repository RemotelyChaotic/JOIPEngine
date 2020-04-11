#ifndef KINKTREEMODEL_H
#define KINKTREEMODEL_H

#include <QAbstractItemModel>
#include <memory>

class CDatabaseManager;
class CKinkTreeItem;

class CKinkTreeModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  explicit CKinkTreeModel(QObject* pParent = nullptr);
  ~CKinkTreeModel() override;

  void Initialize();
  void DeInitializeModel();

  QVariant data(const QModelIndex& index, int iRole) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  QVariant headerData(int iSection, Qt::Orientation orientation,
                      int iRole = Qt::DisplayRole) const override;
  QModelIndex index(int iRow, int iColumn,
                    const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& index) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;

private:
  CKinkTreeItem* GetItem(const QModelIndex& index) const;

  std::weak_ptr<CDatabaseManager> m_wpDbManager;
  CKinkTreeItem*                  m_pRootItem;
};

#endif // KINKTREEMODEL_H
