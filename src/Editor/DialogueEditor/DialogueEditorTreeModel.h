#ifndef CDIALOGueEDITORTREEMODEL_H
#define CDIALOGueEDITORTREEMODEL_H

#include "Systems/DialogueTree.h"
#include "Systems/Database/Project.h"

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

class CDatabaseManager;
class CDialogueEditorTreeItem;
class QUndoStack;
struct CDialogueNode;

class CDialogueEditorTreeModel : public QAbstractItemModel
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

  CDialogueEditorTreeModel(QObject* pParent = nullptr);
  ~CDialogueEditorTreeModel() override;

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
  bool insertRow(qint32 iPosition, EDialogueTreeNodeType type,
                 const QString& sName,
                 const QModelIndex& parent = QModelIndex());
  bool insertNode(qint32 iPosition,
                  const std::shared_ptr<CDialogueNode>& spNode,
                  const QModelIndex& parent = QModelIndex());
  bool removeRow(qint32 iPosition,
                 const QModelIndex& parent = QModelIndex());

  // convenience-functions
  bool HasCondition(const QModelIndex& index = QModelIndex()) const;
  QModelIndex Index(const QStringList& vsPath) const;
  bool IsCategoryType(const QModelIndex& index = QModelIndex()) const;
  bool IsDialogueType(const QModelIndex& index = QModelIndex()) const;
  bool IsDialogueFragmentType(const QModelIndex& index = QModelIndex()) const;
  QStringList Path(QModelIndex idx) const;
  std::shared_ptr<CDialogueNode> Node(QModelIndex idx) const;
  std::shared_ptr<CDialogueNode> Root() const;
  void UpdateFrom(const QModelIndex& index, const std::shared_ptr<CDialogueNode>& spNode);

signals:
  void SignalProjectEdited();

protected:
  CDialogueEditorTreeItem* GetItem(const QModelIndex& index) const;
  QString GetToolTip(const QModelIndex& index) const;
  void BuildTreeItems(CDialogueEditorTreeItem* pLocalRoot,
                      const std::shared_ptr<CDialogueNode>& spNode);

private slots:
  void SlotResourceAdded(qint32 iProjId, const QString& sName);
  void SlotResourceRemoved(qint32 iProjId, const QString& sName);

private:
  std::weak_ptr<CDatabaseManager>             m_wpDbManager;
  tspProject                                  m_spProject;
  std::shared_ptr<CDialogueNode>                m_spDataRootNode;
  CDialogueEditorTreeItem*                      m_pRootItem = nullptr;
};

#endif // CDIALOGueEDITORTREEMODEL_H
