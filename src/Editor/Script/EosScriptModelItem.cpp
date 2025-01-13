#include "EosScriptModelItem.h"
#include "EosCommandModels.h"

SItemIndexPath::SItemIndexPath() :
  m_viRowPath(),
  m_iRole(-1),
  m_iColumn(-1),
  m_sName(),
  m_type(EosScriptModelItem::eRoot)
{
}

//----------------------------------------------------------------------------------------
//
bool operator==(const SItemIndexPath& lhs, const SItemIndexPath& rhs)
{
  return lhs.m_viRowPath == rhs.m_viRowPath &&
      lhs.m_iColumn == rhs.m_iColumn &&
      lhs.m_iRole == rhs.m_iRole &&
      lhs.m_sName == rhs.m_sName &&
      lhs.m_type == rhs.m_type;
}
bool operator!=(const SItemIndexPath& lhs, const SItemIndexPath& rhs)
{
  return !(lhs == rhs);
}

//----------------------------------------------------------------------------------------
//
CEosScriptModelItem::CEosScriptModelItem(EosScriptModelItem type,
                                         CEosScriptModelItem* pParentItem,
                                         const std::shared_ptr<CJsonInstructionNode>& spInstruction) :
  m_spInstruction(spInstruction),
  m_pParentItem(pParentItem),
  m_type(type)
{
}

CEosScriptModelItem::~CEosScriptModelItem()
{
  qDeleteAll(m_vpChildItems);
}

//----------------------------------------------------------------------------------------
//
CEosScriptModelItem* CEosScriptModelItem::DataCopy() const
{
  CEosScriptModelItem* pCopy = new CEosScriptModelItem(m_type, m_pParentItem, m_spInstruction);
  pCopy->SetCheckedWithoutNodeInteraction(m_bChecked);
  pCopy->SetCustomName(m_sCustomName);
  return pCopy;
}

//----------------------------------------------------------------------------------------
//
tInstructionMapValue* CEosScriptModelItem::Arguments() const
{
  if (nullptr != m_spInstruction)
  {
    return &m_spInstruction->m_actualArgs;
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
void CEosScriptModelItem::AppendChild(CEosScriptModelItem* pItem)
{
  m_vpChildItems.append(pItem);
}

//----------------------------------------------------------------------------------------
//
CEosScriptModelItem* CEosScriptModelItem::Child(qint32 iRow) const
{
  if (iRow < 0 || iRow >= m_vpChildItems.size())
  {
    return nullptr;
  }
  return m_vpChildItems.at(iRow);
}

//----------------------------------------------------------------------------------------
//
qint32 CEosScriptModelItem::ChildIndex(CEosScriptModelItem* pCompare) const
{
  for (qint32 i = 0; ChildCount() > i; ++i)
  {
    if (nullptr != pCompare && pCompare == m_vpChildItems[i])
    {
      return i;
    }
  }
  return -1;
}

//----------------------------------------------------------------------------------------
//
qint32 CEosScriptModelItem::ChildCount() const
{
  return m_vpChildItems.count();
}

//----------------------------------------------------------------------------------------
//
qint32 CEosScriptModelItem::ColumnCount() const
{
  return eos_item::c_iNumColumns;
}

//----------------------------------------------------------------------------------------
//
QVariant CEosScriptModelItem::Data(qint32 iColumn, qint32 iRole) const
{
  if (eos_item::c_iNumColumns <= iColumn || 0 > iColumn) { return QVariant(); }

  switch(iColumn)
  {
    case eos_item::c_iColumnName:
    {
      switch (iRole)
      {
        case Qt::EditRole: [[fallthrough]];
        case Qt::DisplayRole:
        {
          return DisplayName();
        }
        case Qt::DecorationRole: return QVariant();
        case Qt::CheckStateRole:
        {
          if (EosScriptModelItem::eInstruction == m_type._to_integral())
          {
            return m_bChecked ? Qt::Checked : Qt::Unchecked;
          }
          return QVariant();
        }
        case eos_item::c_iRoleEosItemType: return m_spInstruction->m_sName;
        default: break;
      }
    } break;
    case eos_item::c_iColumnType:
    {
      switch (iRole)
      {
        case Qt::EditRole: [[fallthrough]];
        case Qt::DisplayRole:
        {
          return QString(m_type._to_string());
        }
        case eos_item::c_iRoleEosItemType: return m_spInstruction->m_sName;
        default: break;
      }
    } break;
  }

  return QVariant();
}

//----------------------------------------------------------------------------------------
//
QString CEosScriptModelItem::DisplayName() const
{
  if (nullptr != m_spInstruction)
  {
    IEosCommandModel* pCommandModel =
        dynamic_cast<IEosCommandModel*>(m_spInstruction->m_wpCommand.lock().get());
    if (nullptr != pCommandModel)
    {
      return m_sCustomName.isEmpty() ? pCommandModel->DisplayName(m_spInstruction->m_actualArgs) :
                                       m_sCustomName;
    }
    return m_sCustomName.isEmpty() ? m_spInstruction->m_sName : m_sCustomName;
  }
  else
  {
    return m_sCustomName;
  }
}

//----------------------------------------------------------------------------------------
//
QString CEosScriptModelItem::Name() const
{
  if (nullptr != m_spInstruction)
  {
    return m_sCustomName.isEmpty() ? m_spInstruction->m_sName : m_sCustomName;
  }
  else
  {
    return m_sCustomName;
  }
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CJsonInstructionNode> CEosScriptModelItem::Node() const
{
  return m_spInstruction;
}

//----------------------------------------------------------------------------------------
//
bool CEosScriptModelItem::InsertChild(CEosScriptModelItem* pChild, qint32 iRow)
{
  if (iRow < 0 || iRow > m_vpChildItems.size())
  {
    return false;
  }

  m_vpChildItems.insert(iRow, pChild);
  return true;
}

//----------------------------------------------------------------------------------------
//
bool CEosScriptModelItem::InsertColumns(qint32 iPosition, qint32 iColumns)
{
  Q_UNUSED(iPosition)
  Q_UNUSED(iColumns)
  return true;
}

//----------------------------------------------------------------------------------------
//
bool CEosScriptModelItem::IsChecked() const
{
  return m_bChecked;
}

//----------------------------------------------------------------------------------------
//
bool CEosScriptModelItem::RemoveChildren(qint32 iPosition, qint32 iCount)
{
  if (iPosition < 0 || iPosition + iCount > m_vpChildItems.size())
  {
    return false;
  }

  for (qint32 iRow = 0; iRow < iCount; ++iRow)
  {
    delete m_vpChildItems.takeAt(iPosition);
  }

  return true;
}

//----------------------------------------------------------------------------------------
//
bool CEosScriptModelItem::RemoveColumns(qint32 iPosition, qint32 iColumns)
{
  Q_UNUSED(iPosition)
  Q_UNUSED(iColumns)
  return true;
}

//----------------------------------------------------------------------------------------
//
CEosScriptModelItem* CEosScriptModelItem::Parent() const
{
  return m_pParentItem;
}

//----------------------------------------------------------------------------------------
//
qint32 CEosScriptModelItem::Row() const
{
  if (m_pParentItem)
  {
    return m_pParentItem->m_vpChildItems.indexOf(const_cast<CEosScriptModelItem*>(this));
  }
  return 0;
}

//----------------------------------------------------------------------------------------
//
void CEosScriptModelItem::SetCustomName(const QString& sName)
{
  m_sCustomName = sName;
}

//----------------------------------------------------------------------------------------
//
void CEosScriptModelItem::SetChecked(bool bValue)
{
  if (m_bChecked != bValue)
  {
    SetCheckedWithoutNodeInteraction(bValue);
    if (!bValue)
    {
      if (auto spParent = m_spInstruction->m_wpParent.lock())
      {
        auto it =
            std::find(spParent->m_spChildren.begin(), spParent->m_spChildren.end(),
                      m_spInstruction);
        if (spParent->m_spChildren.end() != it)
        {
          std::shared_ptr<CJsonInstructionNode> spNoop =
              std::make_shared<CJsonInstructionNode>();
          spNoop->m_sName = eos::c_sCommandNoop;
          spNoop->m_wpParent = spParent;
          spNoop->m_spChildren.push_back(m_spInstruction);

          m_spInstruction->m_wpParent = spNoop;

          qint32 iIndex = it - spParent->m_spChildren.begin();
          spParent->m_spChildren.erase(it);
          spParent->m_spChildren.insert(spParent->m_spChildren.begin()+iIndex, spNoop);
        }
        else
        {
          assert(false);
        }
      }
      else
      {
        assert(false);
      }
    }
    else
    {
      if (auto spNoopParent = m_spInstruction->m_wpParent.lock())
      {
        if (auto spGrandParent = spNoopParent->m_wpParent.lock())
        {
          m_spInstruction->m_wpParent = spGrandParent;
          auto it =
              std::find(spGrandParent->m_spChildren.begin(), spGrandParent->m_spChildren.end(),
                        spNoopParent);
          if (spGrandParent->m_spChildren.end() != it)
          {
            spNoopParent->m_wpParent = std::weak_ptr<CJsonInstructionNode>();
            spNoopParent->m_spChildren.clear();

            qint32 iIndex = it - spGrandParent->m_spChildren.begin();
            spGrandParent->m_spChildren.erase(it);
            spGrandParent->m_spChildren.insert(spGrandParent->m_spChildren.begin()+iIndex,
                                               m_spInstruction);
          }
          else
          {
            assert(false);
          }
        }
        else
        {
          assert(false);
        }
      }
      else
      {
        assert(false);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEosScriptModelItem::SetCheckedWithoutNodeInteraction(bool bValue)
{
  m_bChecked = bValue;
}

//----------------------------------------------------------------------------------------
//
bool CEosScriptModelItem::SetData(qint32, qint32, const QVariant&)
{
  return false;
}
