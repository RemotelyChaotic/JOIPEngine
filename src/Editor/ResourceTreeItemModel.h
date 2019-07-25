#ifndef RESOURCETREEITEMMODEL_H
#define RESOURCETREEITEMMODEL_H

#include "Backend/Project.h"
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <map>

class CResourceTreeItem;

class CResourceTreeItemModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  explicit CResourceTreeItemModel(QObject* pParent = nullptr);
  ~CResourceTreeItemModel() override;

  void InitializeModel(tspProject spProject);
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
  CResourceTreeItem*                          m_pRootItem;
  std::map<EResourceType, CResourceTreeItem*> m_categoryMap;
  tspProject                                  m_spProject;
};

#endif // RESOURCETREEITEMMODEL_H
