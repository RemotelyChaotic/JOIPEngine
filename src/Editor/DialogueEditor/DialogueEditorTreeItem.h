#ifndef CDIALOGueEDITORTREEITEM_H
#define CDIALOGueEDITORTREEITEM_H

#include "Systems/DialogueTree.h"

#include <QVariant>
#include <QVector>

namespace dialogue_item
{
  [[maybe_unused]] const qint32 c_iColumnId       = 0;
  [[maybe_unused]] const qint32 c_iColumnString   = 1;
  [[maybe_unused]] const qint32 c_iColumnWaitMS   = 2;
  [[maybe_unused]] const qint32 c_iColumnSkippable= 3;
  [[maybe_unused]] const qint32 c_iColumnMedia    = 4;

  [[maybe_unused]] const qint32 c_iNumColumns     = 5;

  [[maybe_unused]] const qint32 c_iColumnCondition     = 5;
  [[maybe_unused]] const qint32 c_iColumnType          = 6;
  [[maybe_unused]] const qint32 c_iColumnResource      = 7;
  [[maybe_unused]] const qint32 c_iColumnWarning       = 8;

  [[maybe_unused]] const qint32 c_iNumItems            = 9;

  [[maybe_unused]] const qint32 c_iColumnDisplayString = 9;

  [[maybe_unused]] const qint32 c_iNumOptItems            = 10;
}

class CDialogueEditorTreeItem
{
public:
  CDialogueEditorTreeItem(std::shared_ptr<CDialogueNode> spNode,
                        CDialogueEditorTreeItem* pParentItem = nullptr);
  ~CDialogueEditorTreeItem();

  void AppendChild(CDialogueEditorTreeItem* pChild);
  QString Name() const { return m_spNode->m_sName; }
  std::shared_ptr<CDialogueNode> Node() const { return m_spNode; };
  EDialogueTreeNodeType Type() const { return m_spNode->m_type; }
  void SetNode(const std::shared_ptr<CDialogueNode>& spNode) { m_spNode = spNode; }
  void SetWarning(const QString& sWarning) { m_sWarning = sWarning; }

  CDialogueEditorTreeItem* Child(qint32 iRow);
  qint32 ChildIndex(CDialogueEditorTreeItem* pCompare);
  qint32 ChildCount() const;
  qint32 ColumnCount() const;
  QVariant Data(qint32 iColumn) const;
  Qt::ItemFlags Flags(qint32 iColumn) const;
  QVariant HeaderData(qint32 iColumn) const;
  bool InsertChildren(qint32 iPosition, qint32 iCount, qint32 iColumns);
  bool InsertColumns(qint32 iPosition, qint32 iColumns);
  bool RemoveChildren(qint32 iPosition, qint32 iCount);
  bool RemoveColumns(qint32 iPosition, qint32 iColumns);
  CDialogueEditorTreeItem* Parent();
  qint32 Row() const;
  bool SetData(qint32 iColumn, const QVariant &value);

private:
  CDialogueEditorTreeItem*          m_pParentItem;
  QVector<CDialogueEditorTreeItem*> m_vpChildItems;
  std::shared_ptr<CDialogueNode>    m_spNode;
  QString                           m_sWarning;
};

#endif // CDIALOGueEDITORTREEITEM_H
