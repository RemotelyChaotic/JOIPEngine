#ifndef CDIALOGEDITORTREEMODEL_H
#define CDIALOGEDITORTREEMODEL_H

#include "Systems/DialogTree.h"
#include "Systems/Project.h"

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

class CDatabaseManager;
class CDialogEditorTreeItem;
class QUndoStack;
struct CDialogNode;

class CDialogEditorTreeModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  enum ECutomRoles
  {
    eSearchRole = Qt::UserRole,
    eConditionRole = Qt::UserRole+1,
    eTypeRole = Qt::UserRole+2,
    eResourceRole = Qt::UserRole+3,
    eItemWarningRole = Qt::UserRole+4
  };

  CDialogEditorTreeModel(QObject* pParent = nullptr);
  ~CDialogEditorTreeModel() override;

  void InitializeModel(tspProject spProject);
  void DeInitializeModel();

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
  bool setHeaderData(qint32 iSection, Qt::Orientation orientation,
                     const QVariant& value, qint32 iRole = Qt::EditRole) override;
  bool insertRow(qint32 iPosition, EDialogTreeNodeType type,
                 const QString& sName,
                 const QModelIndex& parent = QModelIndex());
  bool insertNode(qint32 iPosition,
                  const std::shared_ptr<CDialogNode>& spNode,
                  const QModelIndex& parent = QModelIndex());
  bool removeRow(qint32 iPosition,
                 const QModelIndex& parent = QModelIndex());

  // convenience-functions
  bool HasCondition(const QModelIndex& index = QModelIndex()) const;
  QModelIndex Index(const QStringList& vsPath) const;
  bool IsCategoryType(const QModelIndex& index = QModelIndex()) const;
  bool IsDialogType(const QModelIndex& index = QModelIndex()) const;
  bool IsDialogFragmentType(const QModelIndex& index = QModelIndex()) const;
  QStringList Path(QModelIndex idx) const;
  std::shared_ptr<CDialogNode> Node(QModelIndex idx) const;
  std::shared_ptr<CDialogNode> Root() const;
  void UpdateFrom(const QModelIndex& index, const std::shared_ptr<CDialogNode>& spNode);

signals:
  void SignalProjectEdited();

protected:
  CDialogEditorTreeItem* GetItem(const QModelIndex& index) const;
  QString GetToolTip(const QModelIndex& index) const;
  void BuildTreeItems(CDialogEditorTreeItem* pLocalRoot,
                      const std::shared_ptr<CDialogNode>& spNode);

private slots:
  void SlotResourceAdded(qint32 iProjId, const QString& sName);
  void SlotResourceRemoved(qint32 iProjId, const QString& sName);

private:
  std::weak_ptr<CDatabaseManager>             m_wpDbManager;
  tspProject                                  m_spProject;
  std::shared_ptr<CDialogNode>                m_spDataRootNode;
  CDialogEditorTreeItem*                      m_pRootItem = nullptr;
};

#endif // CDIALOGEDITORTREEMODEL_H
