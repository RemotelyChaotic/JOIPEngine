#ifndef CJSONINSTRUCTIONSETRUNNERITEMMODEL_H
#define CJSONINSTRUCTIONSETRUNNERITEMMODEL_H

#include "JsonInstructionSetRunner.h"

#include <QAbstractItemModel>

#include <memory>

class CJsonInstructionNode;

class CJsonInstructionSetRunnerItemModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  explicit CJsonInstructionSetRunnerItemModel(QObject* pParent = nullptr);
  ~CJsonInstructionSetRunnerItemModel() override;

  void SetRunner(const std::shared_ptr<CJsonInstructionSetRunner>& spRunner);

  // read-only functions
  QVariant data(const QModelIndex& index, int iRole, int iColumnOverride) const;
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
  bool insertRows(qint32 iPosition, qint32 iRows,
                  const QModelIndex& parent = QModelIndex()) override;
  bool removeRows(qint32 iPosition, qint32 iRows,
                  const QModelIndex& parent = QModelIndex()) override;

  CJsonInstructionNode* GetItem(const QModelIndex& index) const;

signals:

private:
  std::shared_ptr<CJsonInstructionSetRunner> m_spRunner;
};

#endif // CJSONINSTRUCTIONSETRUNNERITEMMODEL_H
