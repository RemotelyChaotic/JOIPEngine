#ifndef CEOSSCRIPTMODEL_H
#define CEOSSCRIPTMODEL_H

#include "Systems/JSON/JsonInstructionSetRunnerItemModel.h"

#include <QSortFilterProxyModel>

class CEosScriptModelItem;
class CEosScriptModel : public QAbstractItemModel
{
public:
  explicit CEosScriptModel(QObject* pParent = nullptr);
  ~CEosScriptModel() override;

  void InsertInstruction(const QModelIndex& current,
                         const QString& sType, const tInstructionMapValue& args);
  void Invalidate(const QModelIndex& idx);
  void RemoveInstruction(const QModelIndex& current);
  void SetRunner(const std::shared_ptr<CJsonInstructionSetRunner>& spRunner);
  std::shared_ptr<CJsonInstructionSetRunner> Runner() const;
  void Update(const QModelIndex& idx);

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
  bool insertRows(qint32 iPosition, qint32 iRows,
                  const QModelIndex& parent = QModelIndex()) override;
  bool removeRows(qint32 iPosition, qint32 iRows,
                  const QModelIndex& parent = QModelIndex()) override;

  CEosScriptModelItem* GetItem(const QModelIndex& index) const;

signals:

protected:
  const std::vector<std::pair<QString, std::shared_ptr<CJsonInstructionNode>>>& RootNodes() const;

private:
  void Inserted(const QModelIndex& parent, qint32 iIndex);
  void RecursivelyConstruct(CEosScriptModelItem* pParentItem,
                            std::shared_ptr<CJsonInstructionNode> spParentInstruction);

  std::shared_ptr<CJsonInstructionSetRunner>            m_spRunner;
  CEosScriptModelItem*                                  m_pRoot = nullptr;
  std::vector<std::pair<QString, CEosScriptModelItem*>> m_vRootItems;
};

//----------------------------------------------------------------------------------------
//
class CEosSortFilterProxyModel : public QSortFilterProxyModel
{
public:
  explicit CEosSortFilterProxyModel(QObject* pParent = nullptr);
  ~CEosSortFilterProxyModel() override;

protected:
  bool filterAcceptsRow(int iSourceRow, const QModelIndex& sourceParent) const override;
  void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
};

#endif // CEOSSCRIPTMODEL_H
