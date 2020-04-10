#ifndef RESOURCETREEITEMMODEL_H
#define RESOURCETREEITEMMODEL_H

#include "Systems/Project.h"
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <map>

class CDatabaseManager;
class CResourceTreeItem;

class CResourceTreeItemModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  explicit CResourceTreeItemModel(QObject* pParent = nullptr);
  ~CResourceTreeItemModel() override;

  void InitializeModel(tspProject spProject);
  void DeInitializeModel();

  // read-only functions
  QVariant data(const QModelIndex& index, int iRole, int iColumnOverride);
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
  bool setHeaderData(qint32 iSection, Qt::Orientation orientation,
                     const QVariant& value, qint32 iRole = Qt::EditRole) override;
  bool insertColumns(qint32 iPosition, qint32 iColumns,
                     const QModelIndex& parent = QModelIndex()) override;
  bool removeColumns(qint32 iPosition, qint32 iColumns,
                     const QModelIndex& parent = QModelIndex()) override;
  bool insertRows(qint32 iPosition, qint32 iRows,
                  const QModelIndex& parent = QModelIndex()) override;
  bool removeRows(qint32 iPosition, qint32 iRows,
                  const QModelIndex& parent = QModelIndex()) override;

  // convenience-function
  bool IsResourceType(const QModelIndex& index = QModelIndex());

signals:
  void SignalProjectEdited();

private slots:
  void SlotResourceAdded(qint32 iProjId, const QString& sName);
  void SlotResourceRemoved(qint32 iProjId, const QString& sName);

private:
  CResourceTreeItem* GetItem(const QModelIndex& index) const;

  std::weak_ptr<CDatabaseManager>             m_wpDbManager;
  CResourceTreeItem*                          m_pRootItem;
  std::map<EResourceType, CResourceTreeItem*> m_categoryMap;
  tspProject                                  m_spProject;
};

#endif // RESOURCETREEITEMMODEL_H
