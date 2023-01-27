#ifndef CEOSSCRIPTMODELITEM_H
#define CEOSSCRIPTMODELITEM_H

#include "Systems/JSON/JsonInstructionNode.h"

#include <enum.h>

#include <QtGlobal>
#include <QVariant>

#include <vector>

BETTER_ENUM(EosScriptModelItem, qint32,
            eRoot            = 0,
            eInstructionSet  = 1,
            eInstruction     = 2,
            eInstructionChild= 3);

namespace eos_item
{
  const qint32 c_iColumnName       = 0;
  const qint32 c_iColumnType       = 1;

  const qint32 c_iNumColumns       = 2;


  const qint32 c_iRoleEosItemType = Qt::UserRole;
}

struct SItemIndexPath
{
  SItemIndexPath();

  std::vector<qint32> m_viRowPath;
  qint32 m_iRole;
  qint32 m_iColumn;
  QString m_sName;
  EosScriptModelItem m_type;
};
bool operator==(const SItemIndexPath& lhs, const SItemIndexPath& rhs);
bool operator!=(const SItemIndexPath& lhs, const SItemIndexPath& rhs);

class CEosScriptModelItem
{
public:
  CEosScriptModelItem(EosScriptModelItem type, CEosScriptModelItem* pParentItem,
                      const std::shared_ptr<CJsonInstructionNode>& spInstruction);
  ~CEosScriptModelItem();

  CEosScriptModelItem* DataCopy() const;

  tInstructionMapValue* Arguments() const;
  EosScriptModelItem Type() { return m_type; }

  void AppendChild(CEosScriptModelItem* pChild);
  CEosScriptModelItem* Child(qint32 iRow) const;
  qint32 ChildIndex(CEosScriptModelItem* pCompare) const;
  qint32 ChildCount() const;
  qint32 ColumnCount() const;
  QVariant Data(qint32 iColumn, qint32 iRole) const;
  QString DisplayName() const;
  QString Name() const;
  std::shared_ptr<CJsonInstructionNode> Node() const;
  bool InsertChild(CEosScriptModelItem* pChild, qint32 iRow);
  bool InsertColumns(qint32 iPosition, qint32 iColumns);
  bool IsChecked() const;
  bool RemoveChildren(qint32 iPosition, qint32 iCount);
  bool RemoveColumns(qint32 iPosition, qint32 iColumns);
  CEosScriptModelItem* Parent() const;
  qint32 Row() const;
  void SetCustomName(const QString& sName);
  void SetChecked(bool bValue);
  void SetCheckedWithoutNodeInteraction(bool bValue);
  bool SetData(qint32 iColumn, qint32 iRole, const QVariant &value);

private:
  std::shared_ptr<CJsonInstructionNode> m_spInstruction;
  CEosScriptModelItem*          m_pParentItem;
  QVector<CEosScriptModelItem*> m_vpChildItems;
  EosScriptModelItem            m_type;
  QString                       m_sCustomName;
  bool                          m_bChecked = true;
};

#endif // CEOSSCRIPTMODELITEM_H
