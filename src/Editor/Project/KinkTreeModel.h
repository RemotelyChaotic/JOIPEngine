#ifndef KINKTREEMODEL_H
#define KINKTREEMODEL_H

#include "Systems/Kink.h"
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

  void InitializeModel();
  void DeInitializeModel();
  void ResetSelections();
  void ResetSelections(QStringList vsUnselectedKinks);
  void SetSelections(QStringList vsSelectedKinks);
  std::vector<tspKink> SelectedItems();

  void SetReadOnly(bool bReadOnly) { m_bReadOnly = bReadOnly; }

  // read-only functions
  QVariant data(const QModelIndex& index, int iRole) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  QVariant headerData(int iSection, Qt::Orientation orientation,
                      int iRole = Qt::DisplayRole) const override;
  QModelIndex index(int iRow, int iColumn,
                    const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& index) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;

  // write functions
  bool setData(const QModelIndex& index, const QVariant& value,
               qint32 iRole = Qt::EditRole) override;

signals:
  void SignalCheckedItem(const QModelIndex& index, bool bChecked);

private slots:
  void SlotDbReloadFinished();

private:
  CKinkTreeItem* GetItem(const QModelIndex& index) const;

  std::weak_ptr<CDatabaseManager> m_wpDbManager;
  CKinkTreeItem*                  m_pRootItem;
  bool                            m_bReadOnly;
};

#endif // KINKTREEMODEL_H
