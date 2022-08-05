#include "JsonInstructionSetRunnerItemModel.h"
#include "JsonInstructionNode.h"

CJsonInstructionSetRunnerItemModel::CJsonInstructionSetRunnerItemModel(QObject* pParent)
  : QAbstractItemModel{pParent}
{

}
CJsonInstructionSetRunnerItemModel::~CJsonInstructionSetRunnerItemModel()
{}

//----------------------------------------------------------------------------------------
//
void CJsonInstructionSetRunnerItemModel::SetRunner(const std::shared_ptr<CJsonInstructionSetRunner>& spRunner)
{
  beginResetModel();
  m_spRunner = spRunner;
  endResetModel();
}

// read-only functions
//----------------------------------------------------------------------------------------
//
QVariant CJsonInstructionSetRunnerItemModel::data(const QModelIndex& index, int iRole, int iColumnOverride) const
{
  Q_UNUSED(iColumnOverride)
  const CJsonInstructionNode* pItem = GetItem(index);
  if (nullptr != pItem)
  {
    switch (iRole)
    {
      case Qt::DisplayRole: [[fallthrough]];
      case Qt::EditRole:
      {
        return pItem->m_sName;
      }
      case Qt::CheckStateRole:
      {
        return pItem->m_bEnabled ? Qt::Checked : Qt::Unchecked;
      }
      default: break;
    }
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
QVariant CJsonInstructionSetRunnerItemModel::data(const QModelIndex& index, int iRole) const
{
  return data(index, iRole, -1);
}

//----------------------------------------------------------------------------------------
//
Qt::ItemFlags CJsonInstructionSetRunnerItemModel::flags(const QModelIndex& index) const
{
  if (!index.isValid()) { return Qt::NoItemFlags; }
  Qt::ItemFlags flags = QAbstractItemModel::flags(index);
  const CJsonInstructionNode* pItem = GetItem(index);
  if (nullptr != pItem && !pItem->m_bEnabled)
  {
    flags &= ~Qt::ItemFlag::ItemIsEnabled;
  }
  return flags;
}

//----------------------------------------------------------------------------------------
//
QVariant CJsonInstructionSetRunnerItemModel::headerData(int iSection, Qt::Orientation orientation,
                                                        int iRole) const
{
  if (Qt::Horizontal == orientation && Qt::DisplayRole == iRole)
  {
    switch (iSection)
    {
      case 0: return "Action";
      default: return QVariant();
    }
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
QModelIndex CJsonInstructionSetRunnerItemModel::index(int iRow, int iColumn,
                                                      const QModelIndex& parent) const
{
  // hasIndex checks if the values are in the valid ranges by using
  // rowCount and columnCount
  if (!hasIndex(iRow, iColumn, parent)) { return QModelIndex(); }

  CJsonInstructionNode* pspParentNode = GetItem(parent);
  if (nullptr == pspParentNode)
  {
    if (nullptr != m_spRunner)
    {
      const auto& nodes = m_spRunner->Nodes();
      if (0 <= iRow && nodes.size() > iRow)
      {
        CJsonInstructionNode* pspChildItem =
            const_cast<CJsonInstructionNode*>(nodes[iRow].second.get());
        return createIndex(iRow, iColumn, pspChildItem);
      }
    }
    return QModelIndex();
  }
  else
  {
    CJsonInstructionNode* pChildItem = pspParentNode->m_spChildren[iRow].get();
    return createIndex(iRow, iColumn, pChildItem);
  }
}

//----------------------------------------------------------------------------------------
//
QModelIndex CJsonInstructionSetRunnerItemModel::parent(const QModelIndex& index) const
{
  if (!index.isValid()) { return QModelIndex(); }
  const CJsonInstructionNode* pChild = GetItem(index);
  if (nullptr != pChild)
  {
    if (auto spParent = pChild->m_wpParent.lock(); nullptr != spParent)
    {
      for (qint32 i = 0; static_cast<qint32>(spParent->m_spChildren.size()) > i; ++i)
      {
        if (spParent->m_spChildren[i].get() == pChild)
        {
          return createIndex(i, 0, spParent.get());
        }
      }
    }
  }
  return QModelIndex();
}

//----------------------------------------------------------------------------------------
//
int CJsonInstructionSetRunnerItemModel::rowCount(const QModelIndex& parent) const
{
  if (!parent.isValid())
  {
    if (nullptr != m_spRunner)
    {
      const auto& nodes = m_spRunner->Nodes();
      return static_cast<qint32>(nodes.size());
    }
    else
    {
      return 0;
    }
  }

  const CJsonInstructionNode* pParentItem = GetItem(parent);
  return static_cast<qint32>(pParentItem->m_spChildren.size());
}

//----------------------------------------------------------------------------------------
//
int CJsonInstructionSetRunnerItemModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent)
  return 1;
}

// write functions
//----------------------------------------------------------------------------------------
//
bool CJsonInstructionSetRunnerItemModel::setData(const QModelIndex& index, const QVariant& value,
                                                 qint32 iRole)
{
  Q_UNUSED(index)
  Q_UNUSED(value)
  Q_UNUSED(iRole)
  return false;
}

//----------------------------------------------------------------------------------------
//
bool CJsonInstructionSetRunnerItemModel::insertRows(qint32 iPosition, qint32 iRows,
                                                    const QModelIndex& parent)
{
  Q_UNUSED(iPosition)
  Q_UNUSED(iRows)
  Q_UNUSED(parent)
  return false;
}

//----------------------------------------------------------------------------------------
//
bool CJsonInstructionSetRunnerItemModel::removeRows(qint32 iPosition, qint32 iRows,
                                                    const QModelIndex& parent)
{
  Q_UNUSED(iPosition)
  Q_UNUSED(iRows)
  Q_UNUSED(parent)
  return false;
}

//----------------------------------------------------------------------------------------
//
CJsonInstructionNode* CJsonInstructionSetRunnerItemModel::GetItem(const QModelIndex& index) const
{
  if (index.isValid())
  {
    CJsonInstructionNode* pItem = static_cast<CJsonInstructionNode*>(index.internalPointer());
    if (nullptr != pItem) { return pItem; }
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
const std::vector<std::pair<QString, std::shared_ptr<CJsonInstructionNode>>>&
CJsonInstructionSetRunnerItemModel::RootNodes() const
{
  if (nullptr != m_spRunner)
  {
    return m_spRunner->Nodes();
  }
  static std::vector<std::pair<QString, std::shared_ptr<CJsonInstructionNode>>> m_empty;
  return m_empty;
}
