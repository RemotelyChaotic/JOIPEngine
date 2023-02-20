#include "EosScriptModel.h"
#include "CommandInsertEosCommand.h"
#include "CommandRemoveEosCommand.h"
#include "CommandToggledEosCommand.h"
#include "EosScriptModelItem.h"
#include "EosCommandModels.h"

#include "Systems/JSON/JsonInstructionNode.h"

#include "Systems/EOS/EosCommands.h"

#include <QUndoStack>
#include <variant>

CEosScriptModel::CEosScriptModel(QObject* pParent) :
  QAbstractItemModel(pParent),
  m_pRoot(new CEosScriptModelItem(EosScriptModelItem::eRoot, nullptr, nullptr)),
  m_vRootItems()
{
  m_pRoot->SetCustomName("Root");
}

CEosScriptModel::~CEosScriptModel()
{
}

//----------------------------------------------------------------------------------------
//
void CEosScriptModel::InsertInstruction(const QModelIndex& current,
                                        const QString& sType, const tInstructionMapValue& args)
{
  if (nullptr != m_pUndoStack)
  {
    SItemIndexPath path;
    GetIndexPath(current, -1, &path);
    m_pUndoStack->push(new CCommandInsertEosCommand(this, path, sType, args));
  }
  else
  {
    InsertInstructionImpl(current, sType, args);
  }
}

//----------------------------------------------------------------------------------------
//
void CEosScriptModel::Invalidate(const QModelIndex& idx)
{
  if (idx.isValid())
  {
    QModelIndex currentIdx = idx;
    CEosScriptModelItem* pCurrentItem = GetItem(idx);

    if (nullptr != pCurrentItem)
    {
      if (pCurrentItem->Type()._to_integral() == EosScriptModelItem::eInstructionChild)
      {
        pCurrentItem = pCurrentItem->Parent();
        currentIdx = parent(currentIdx);
      }
      if (pCurrentItem->Type()._to_integral() == EosScriptModelItem::eInstruction)
      {
        if (pCurrentItem->ChildCount() > 0)
        {
          beginRemoveRows(currentIdx, 0, pCurrentItem->ChildCount()-1);
          pCurrentItem->RemoveChildren(0, pCurrentItem->ChildCount());
          endRemoveRows();
        }

        IJsonInstructionBase::tChildNodeGroups vChildGroups;
        if (auto spCommand = pCurrentItem->Node()->m_wpCommand.lock())
        {
          vChildGroups = spCommand->ChildNodeGroups(pCurrentItem->Node()->m_actualArgs);
        }

        if (vChildGroups.size() > 0)
        {
          beginInsertRows(currentIdx, 0, static_cast<qint32>(vChildGroups.size()-1));
          RecursivelyConstruct(pCurrentItem, pCurrentItem->Node());
          endInsertRows();
        }

        Update(currentIdx);
        return;
      }
    }

    Update(idx);
  }
}

//----------------------------------------------------------------------------------------
//
void CEosScriptModel::RemoveInstruction(const QModelIndex& current)
{
  if (nullptr != m_pUndoStack)
  {
    SItemIndexPath path;
    GetIndexPath(current, -1, &path);
    m_pUndoStack->push(new CCommandRemoveEosCommand(this, path));
  }
  else
  {
    RemoveInstructionImpl(current);
  }
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CJsonInstructionSetRunner> CEosScriptModel::Runner() const
{
  return m_spRunner;
}

//----------------------------------------------------------------------------------------
//
void CEosScriptModel::SetRunner(const std::shared_ptr<CJsonInstructionSetRunner>& spRunner)
{
  beginResetModel();
  if (nullptr != m_pRoot)
  {
    m_vRootItems.clear();
    delete m_pRoot;
    m_pRoot = nullptr;
  }

  if (nullptr != spRunner)
  {
    m_spRunner = spRunner;

    m_pRoot = new CEosScriptModelItem(EosScriptModelItem::eRoot, nullptr, nullptr);
    m_pRoot->SetCustomName("Root");

    for (const std::pair<QString, std::shared_ptr<CJsonInstructionNode>>& pair : m_spRunner->Nodes())
    {
      m_vRootItems.push_back(
            {pair.first, new CEosScriptModelItem(
                              EosScriptModelItem::eInstructionSet, m_pRoot, pair.second)});
      m_pRoot->AppendChild(m_vRootItems.back().second);
      m_vRootItems.back().second->SetCustomName(pair.first.isEmpty() ? tr("&lt;Commands&gt;") : pair.first);

      RecursivelyConstruct(m_vRootItems.back().second, pair.second);
    }
  }
  else
  {
    m_pRoot = new CEosScriptModelItem(EosScriptModelItem::eRoot, nullptr, nullptr);
    m_pRoot->SetCustomName("Root");
  }

  // parse
  endResetModel();
}

//----------------------------------------------------------------------------------------
//
void CEosScriptModel::SetUndoStack(QUndoStack* pStack)
{
  m_pUndoStack = pStack;
}

//----------------------------------------------------------------------------------------
//
void CEosScriptModel::Update(const QModelIndex& index)
{
  if (index.isValid())
  {
    emit dataChanged(index, index);
    emit SignalContentsChange(index.row(), 1, 1);
  }
}

//----------------------------------------------------------------------------------------
//
QVariant CEosScriptModel::data(const QModelIndex& index, int iRole) const
{
  const CEosScriptModelItem* pItem = GetItem(index);
  if (nullptr != pItem)
  {
    return pItem->Data(index.column(), iRole);
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
Qt::ItemFlags CEosScriptModel::flags(const QModelIndex& index) const
{
  if (!index.isValid()) { return Qt::NoItemFlags; }
  Qt::ItemFlags flags = QAbstractItemModel::flags(index) &= ~Qt::ItemIsUserCheckable;
  switch(index.column())
  {
    case eos_item::c_iColumnName:
    {
      CEosScriptModelItem* pItem = GetItem(index);
      if (nullptr != pItem &&
          pItem->Type()._to_integral() == EosScriptModelItem::eInstruction)
      {
        flags |= Qt::ItemIsUserCheckable;
      }
      break;
    }
    case eos_item::c_iColumnType: break;
    default: break;
  }
  return flags;
}

//----------------------------------------------------------------------------------------
//
QVariant CEosScriptModel::headerData(int iSection, Qt::Orientation orientation,
                                     int iRole) const
{
  if (Qt::Horizontal == orientation && Qt::DisplayRole == iRole)
  {
    switch (iSection)
    {
      case eos_item::c_iColumnName: return "Name";
      case eos_item::c_iColumnType: return "Node-Type";
      default: return QVariant();
    }
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
QModelIndex CEosScriptModel::index(int iRow, int iColumn,
                                   const QModelIndex& parent) const
{
  // hasIndex checks if the values are in the valid ranges by using
  // rowCount and columnCount
  if (!hasIndex(iRow, iColumn, parent)) { return QModelIndex(); }

  const CEosScriptModelItem* pParentNode = GetItem(parent);
  if (nullptr != pParentNode)
  {
    CEosScriptModelItem* pChildNode = pParentNode->Child(iRow);
    return createIndex(iRow, iColumn, pChildNode);
  }
  return QModelIndex();
}

//----------------------------------------------------------------------------------------
//
bool CEosScriptModel::hasChildren(const QModelIndex& parent) const
{
  return QAbstractItemModel::hasChildren(parent);
}

//----------------------------------------------------------------------------------------
//
QModelIndex CEosScriptModel::parent(const QModelIndex& index) const
{
  if (!index.isValid()) { return QModelIndex(); }
  const CEosScriptModelItem* pChild = GetItem(index);
  if (nullptr != pChild)
  {
    CEosScriptModelItem* pParent = pChild->Parent();
    if (nullptr != pParent)
    {
      qint32 iRowOfParent = 0;
      CEosScriptModelItem* pParentParent = pParent->Parent();
      if (nullptr != pParentParent)
      {
        iRowOfParent = pParentParent->ChildIndex(pParent);
        return createIndex(iRowOfParent, index.column(), pParent);
      }
      else
      {
        return QModelIndex();
      }
    }
  }
  return QModelIndex();
}

//----------------------------------------------------------------------------------------
//
int CEosScriptModel::rowCount(const QModelIndex& parent) const
{
  const CEosScriptModelItem* pParentItem = GetItem(parent);
  return static_cast<qint32>(pParentItem->ChildCount());
}

//----------------------------------------------------------------------------------------
//
int CEosScriptModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent)
  return eos_item::c_iNumColumns;
}

// write functions
//----------------------------------------------------------------------------------------
//
bool CEosScriptModel::setData(const QModelIndex& index, const QVariant& value,
                              qint32 iRole)
{
  CEosScriptModelItem* pItem = GetItem(index);
  if (nullptr != pItem)
  {
    if (eos_item::c_iNumColumns <= index.column() || 0 > index.column()) { return false; }

    switch(index.column())
    {
      case eos_item::c_iColumnName:
      {
        switch (iRole)
        {
          case Qt::CheckStateRole:
          {
            bool bChecked = value.value<Qt::CheckState>() == Qt::Checked;
            if (nullptr != m_pUndoStack)
            {
              SItemIndexPath path;
              GetIndexPath(index, iRole, &path);
              m_pUndoStack->push(
                    new CCommandToggledEosCommand(this, path, pItem->IsChecked(), bChecked));
            }
            else
            {
              pItem->SetChecked(bChecked);
            }
            return true;
          }
          default: break;
        }
      }
      default: break;
    }
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
bool CEosScriptModel::insertRows(qint32 iPosition, qint32 iRows,
                                 const QModelIndex& parent)
{
  Q_UNUSED(iPosition)
  Q_UNUSED(iRows)
  Q_UNUSED(parent)
  return false;
}

//----------------------------------------------------------------------------------------
//
bool CEosScriptModel::removeRows(qint32 iPosition, qint32 iRows,
                                 const QModelIndex& parent)
{
  Q_UNUSED(iPosition)
  Q_UNUSED(iRows)
  Q_UNUSED(parent)
  return false;
}

//----------------------------------------------------------------------------------------
//
CEosScriptModelItem* CEosScriptModel::GetItem(const QModelIndex& index) const
{
  if (index.isValid())
  {
    CEosScriptModelItem* pItem = static_cast<CEosScriptModelItem*>(index.internalPointer());
    if (nullptr != pItem) { return pItem; }
  }
  return m_pRoot;
}

//----------------------------------------------------------------------------------------
//
CEosScriptModelItem* CEosScriptModel::GetItem(const SItemIndexPath& path) const
{
  CEosScriptModelItem* pItem = m_pRoot;
  for (qint32 iElem : path.m_viRowPath)
  {
    pItem = pItem->Child(iElem);
    if (nullptr == pItem) { return nullptr; }
  }

  if (pItem->Name() == path.m_sName &&
      pItem->Type() == path.m_type)
  {
    return pItem;
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
bool CEosScriptModel::GetIndexPath(QModelIndex idx, qint32 iRole, SItemIndexPath* pOutPath) const
{
  if (nullptr != pOutPath)
  {
    CEosScriptModelItem* pItem = GetItem(idx);
    CEosScriptModelItem* pChild = pItem;
    CEosScriptModelItem* pParent = pItem->Parent();
    pOutPath->m_viRowPath.clear();
    while (nullptr != pParent)
    {
      qint32 iIndex = pParent->ChildIndex(pChild);
      pOutPath->m_viRowPath.insert(pOutPath->m_viRowPath.begin(), iIndex);
      pChild = pParent;
      pParent = pParent->Parent();
    }
    pOutPath->m_iRole = iRole;
    pOutPath->m_iColumn = idx.column();
    pOutPath->m_sName = pItem->Name();
    pOutPath->m_type = pItem->Type();

    return true;
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
QModelIndex CEosScriptModel::GetIndex(const SItemIndexPath& index) const
{
  CEosScriptModelItem* pItem = GetItem(index);
  if (nullptr != pItem && index.m_viRowPath.size() > 0)
  {
    return createIndex(index.m_viRowPath.back(), index.m_iColumn, pItem);
  }
  return QModelIndex();
}

//----------------------------------------------------------------------------------------
//
QModelIndex CEosScriptModel::InsertInstructionImpl(
    const QModelIndex& current, const QString& sType, const tInstructionMapValue& args)
{
  QModelIndex changedParent;
  qint32 iInsertPoint = 0;
  qint32 iModelItemInsertPoint = 0;
  QString sChildGroup;
  CEosScriptModelItem* pCurrentItem = GetItem(current);

  // depending on selection wen need to insert the child somewhere
  if (nullptr == pCurrentItem ||
      pCurrentItem->Type()._to_integral() == EosScriptModelItem::eRoot)
  {
    pCurrentItem = m_vRootItems.back().second;
    changedParent = createIndex(static_cast<qint32>(m_vRootItems.size())-1,0,pCurrentItem);
    if (nullptr != pCurrentItem)
    {
      iInsertPoint = pCurrentItem->ChildCount();
      iModelItemInsertPoint = iInsertPoint;
    }
  }
  else if (pCurrentItem->Type()._to_integral() == EosScriptModelItem::eInstruction)
  {
    CEosScriptModelItem* pChild = pCurrentItem;
    pCurrentItem = pCurrentItem->Parent();
    changedParent = parent(current);
    if (pCurrentItem->Type()._to_integral() == EosScriptModelItem::eInstructionChild)
    {
      for (qint32 i = 0; pCurrentItem->ChildIndex(pChild) > i; ++i)
      {
        iInsertPoint += pCurrentItem->Child(i)->ChildCount();
      }
      qint32 iOffset = pCurrentItem->ChildIndex(pChild)+1;
      iInsertPoint += iOffset;
      iModelItemInsertPoint = iOffset;
      sChildGroup = pCurrentItem->Data(eos_item::c_iColumnName, Qt::DisplayRole).toString();
    }
    else
    {
      iInsertPoint = pCurrentItem->ChildIndex(pChild)+1;
      iModelItemInsertPoint = iInsertPoint;
    }
  }
  else if (pCurrentItem->Type()._to_integral() == EosScriptModelItem::eInstructionChild)
  {
    CEosScriptModelItem* pParentItem = pCurrentItem->Parent();
    changedParent = current;
    if (nullptr != pParentItem)
    {
      qint32 iMax = pParentItem->ChildIndex(pCurrentItem);
      for (qint32 i = 0; iMax >= i; ++i)
      {
        iInsertPoint += pParentItem->Child(i)->ChildCount();
      }
      iModelItemInsertPoint = pParentItem->Child(iMax)->ChildCount();
      sChildGroup = pCurrentItem->Data(eos_item::c_iColumnName, Qt::DisplayRole).toString();
    }
  }
  else if (pCurrentItem->Type()._to_integral() == EosScriptModelItem::eInstructionSet)
  {
    changedParent = current;
    iInsertPoint = pCurrentItem->ChildCount();
    iModelItemInsertPoint = iInsertPoint;
  }


  if (nullptr != pCurrentItem)
  {
    return InsertInstructionAtImpl(changedParent, iInsertPoint, iModelItemInsertPoint,
                                   sChildGroup, sType,
                                   pCurrentItem->Type()._to_integral(),
                                   args, pCurrentItem->Node());
  }

  return QModelIndex();
}

//----------------------------------------------------------------------------------------
//
QModelIndex CEosScriptModel::InsertInstructionAtImpl(
    const QModelIndex& parent, qint32 iInsertPoint, qint32 iModelItemInsertPoint,
    const QString& sChildGroup, const QString& sType, qint32 itemType,
    const tInstructionMapValue& args, std::shared_ptr<CJsonInstructionNode> spNodeParent)
{
  // on handled cases start the insertion
  beginInsertRows(parent, iModelItemInsertPoint, iModelItemInsertPoint);
  std::shared_ptr<CJsonInstructionNode> spNewNode = std::make_shared<CJsonInstructionNode>();
  if (spNodeParent->m_spChildren.size() > iInsertPoint && iInsertPoint)
  {
    spNodeParent->m_spChildren.insert(spNodeParent->m_spChildren.begin()+iInsertPoint, spNewNode);
  }
  else if (spNodeParent->m_spChildren.size() == iInsertPoint)
  {
    spNodeParent->m_spChildren.push_back(spNewNode);
  }
  spNewNode->m_wpParent = spNodeParent;
  spNewNode->m_sName = sType;
  spNewNode->m_wpCommand = m_spRunner->Instruction(sType);
  spNewNode->m_actualArgs = args;

  if (auto spCommand = std::dynamic_pointer_cast<IEosCommandModel>(spNewNode->m_wpCommand.lock());
      nullptr != spCommand && spNewNode->m_actualArgs.empty())
  {
    spNewNode->m_actualArgs = spCommand->DefaultArgs();
  }

  // if it's a nested istruction we might need to change the parameters of the parent as well
  if (itemType == EosScriptModelItem::eInstructionChild)
  {
    if (auto spCommand = std::dynamic_pointer_cast<IEosCommandModel>(spNodeParent->m_wpCommand.lock());
        nullptr != spCommand)
    {
      spCommand->InsertedChildAt(&spNodeParent->m_actualArgs, iInsertPoint, sChildGroup, sType);
    }
  }

  QModelIndex idxInserted = Inserted(parent, iInsertPoint, iModelItemInsertPoint);

  endInsertRows();

  emit dataChanged(idxInserted, idxInserted);
  emit SignalContentsChange(idxInserted.row(), 0, 1);

  return idxInserted;
}

//----------------------------------------------------------------------------------------
//
void CEosScriptModel::RemoveInstructionImpl(const QModelIndex& current)
{
  bool bAnythingToRemove = false;
  CEosScriptModelItem* pCurrentItem = GetItem(current);

  // depending on selection we need to remove the child or not
  if (nullptr != pCurrentItem &&
      pCurrentItem->Type()._to_integral() == EosScriptModelItem::eInstruction)
  {
    bAnythingToRemove = true;
  }

  if (bAnythingToRemove)
  {
    QModelIndex parentIdx = parent(current);
    CEosScriptModelItem* pParentItem = GetItem(parentIdx);

    std::shared_ptr<CJsonInstructionNode> spCurrentNode = pCurrentItem->Node();
    std::shared_ptr<CJsonInstructionNode> spParentNode = nullptr;
    qint32 iModelIndex = pCurrentItem->Parent()->ChildIndex(pCurrentItem);
    qint32 iRemovePos = -1;
    QString sChildGroup;

    // depending on selection wen need to insert the child somewhere
    if (nullptr == pParentItem ||
        pParentItem->Type()._to_integral() == EosScriptModelItem::eRoot)
    {
      spParentNode = nullptr;
    }
    else if (pParentItem->Type()._to_integral() == EosScriptModelItem::eInstruction)
    {
      spParentNode = nullptr;
    }
    else if (pParentItem->Type()._to_integral() == EosScriptModelItem::eInstructionChild)
    {
      spParentNode = pParentItem->Node();

      if (nullptr != pParentItem->Parent())
      {
        iRemovePos = 0;
        qint32 iMax = pParentItem->Parent()->ChildIndex(pParentItem);
        for (qint32 i = 0; iMax > i; ++i)
        {
          iRemovePos += pParentItem->Parent()->Child(i)->ChildCount();
        }
        iRemovePos += iModelIndex;
        sChildGroup = pParentItem->Data(eos_item::c_iColumnName, Qt::DisplayRole).toString();
      }
    }
    else if (pParentItem->Type()._to_integral() == EosScriptModelItem::eInstructionSet)
    {
      spParentNode = pParentItem->Node();
      iRemovePos = iModelIndex;
    }

    if (-1 != iRemovePos && nullptr != spParentNode)
    {
      // if it's a nested istruction we might need to change the parameters of the parent as well
      if (pParentItem->Type()._to_integral() == EosScriptModelItem::eInstructionChild)
      {
        if (auto spCommand = std::dynamic_pointer_cast<IEosCommandModel>(spParentNode->m_wpCommand.lock());
            nullptr != spCommand)
        {
          spCommand->RemoveChildAt(&spParentNode->m_actualArgs, iModelIndex, sChildGroup);
        }
      }

      beginRemoveRows(parentIdx, iModelIndex, iModelIndex);
      spParentNode->m_spChildren.erase(spParentNode->m_spChildren.begin()+iRemovePos);
      pCurrentItem->Parent()->RemoveChildren(iModelIndex, 1);
      pCurrentItem = nullptr;
      endRemoveRows();

      emit SignalContentsChange(iModelIndex, 1, 0);
    }
  }
}

//----------------------------------------------------------------------------------------
//
const std::vector<std::pair<QString, std::shared_ptr<CJsonInstructionNode>>>&
CEosScriptModel::RootNodes() const
{
  if (nullptr != m_spRunner)
  {
    return m_spRunner->Nodes();
  }
  static std::vector<std::pair<QString, std::shared_ptr<CJsonInstructionNode>>> m_empty;
  return m_empty;
}

//----------------------------------------------------------------------------------------
//
void RecursivelyConstructChild(CEosScriptModelItem* pParentItem,
                               std::shared_ptr<CJsonInstructionNode> spChildItem,
                               std::function<void(CEosScriptModelItem*,
                                                  std::shared_ptr<CJsonInstructionNode>)> fnRecursivelyConstruct)
{
  std::shared_ptr<CJsonInstructionNode> spChildItemToContinue = nullptr;
  CEosScriptModelItem* pChild = nullptr;
  if (eos::c_sCommandNoop != spChildItem->m_sName)
  {
    pChild =
        new CEosScriptModelItem(EosScriptModelItem::eInstruction, pParentItem,
                                spChildItem);
    pParentItem->AppendChild(pChild);

    spChildItemToContinue = spChildItem;
  }
  else
  {
    if (spChildItem->m_spChildren.size() == 1)
    {
      pChild =
          new CEosScriptModelItem(EosScriptModelItem::eInstruction, pParentItem,
                                  spChildItem->m_spChildren.at(0));
      pChild->SetCheckedWithoutNodeInteraction(false);

      pParentItem->AppendChild(pChild);

      spChildItemToContinue = spChildItem->m_spChildren.at(0);
    }
  }

  if (nullptr != pChild)  {
    fnRecursivelyConstruct(pChild, spChildItemToContinue);
  }
}

//----------------------------------------------------------------------------------------
//
QModelIndex CEosScriptModel::Inserted(const QModelIndex& parent, qint32 iInsertPoint, qint32 iModelItemInsertPoint)
{
  CEosScriptModelItem* pParentItem = GetItem(parent);
  CEosScriptModelItem* pChildItem = nullptr;
  if (nullptr != pParentItem->Node())
  {
    std::shared_ptr<CJsonInstructionNode> spParentNode = pParentItem->Node();
    std::shared_ptr<CJsonInstructionNode> spChildNode = nullptr;

    IJsonInstructionBase::tChildNodeGroups vChildGroups;
    if (auto spCommand = spParentNode->m_wpCommand.lock())
    {
      vChildGroups = spCommand->ChildNodeGroups(spParentNode->m_actualArgs);
    }

    spChildNode = spParentNode->m_spChildren.at(iInsertPoint);
    pChildItem =
        new CEosScriptModelItem(EosScriptModelItem::eInstruction, pParentItem,
                                spChildNode);
    pParentItem->InsertChild(pChildItem, iModelItemInsertPoint);

    RecursivelyConstruct(pChildItem, spChildNode);

    return createIndex(iModelItemInsertPoint, eos_item::c_iColumnName, pChildItem);
  }

  return QModelIndex();
}

//----------------------------------------------------------------------------------------
//
void CEosScriptModel::RecursivelyConstruct(
    CEosScriptModelItem* pParentItem,
    std::shared_ptr<CJsonInstructionNode> spParentInstruction)
{
  IJsonInstructionBase::tChildNodeGroups vChildGroups;
  if (auto spCommand = spParentInstruction->m_wpCommand.lock())
  {
    vChildGroups = spCommand->ChildNodeGroups(spParentInstruction->m_actualArgs);
  }

  // zwischen-nodes einhängen falls nötig
  if (!vChildGroups.empty())
  {
    qint32 iCurrent = 0;
    for (const auto& pair : vChildGroups)
    {
      CEosScriptModelItem* pChild =
                new CEosScriptModelItem(EosScriptModelItem::eInstructionChild, pParentItem,
                                        spParentInstruction);
      pParentItem->AppendChild(pChild);
      pChild->SetCustomName(std::get<0>(pair));

      for (qint32 i = iCurrent; iCurrent+std::get<2>(pair) > i; ++i)
      {
        if (spParentInstruction->m_spChildren.size() <= i) { assert(false); break; }
        RecursivelyConstructChild(pChild, spParentInstruction->m_spChildren[i],
                                  std::bind(&CEosScriptModel::RecursivelyConstruct, this,
                                            std::placeholders::_1, std::placeholders::_2));
      }
      iCurrent += std::get<2>(pair);
    }
  }
  else
  {
    for (const auto& spChildItem : spParentInstruction->m_spChildren)
    {
      RecursivelyConstructChild(pParentItem, spChildItem,
                                std::bind(&CEosScriptModel::RecursivelyConstruct, this,
                                          std::placeholders::_1, std::placeholders::_2));
    }
  }
}

//----------------------------------------------------------------------------------------
//
CEosSortFilterProxyModel::CEosSortFilterProxyModel(QObject* pParent) :
  QSortFilterProxyModel(pParent)
{
}
CEosSortFilterProxyModel::~CEosSortFilterProxyModel()
{}

//----------------------------------------------------------------------------------------
//
bool CEosSortFilterProxyModel::filterAcceptsRow(int iSourceRow,
                                                const QModelIndex& sourceParent) const
{
  CEosScriptModel* pSourceModel = dynamic_cast<CEosScriptModel*>(sourceModel());
  if (nullptr != pSourceModel)
  {
    if(!filterRegExp().isEmpty())
    {
      // get source-model index for current row
      QModelIndex sourceIndex = pSourceModel->index(iSourceRow, 0, sourceParent);
      if(sourceIndex.isValid())
      {
        // if any of children matches the filter, then current index matches the filter as well
        qint32 iNb = pSourceModel->rowCount(sourceIndex) ;
        for(qint32 i = 0; i < iNb; ++i)
        {
          if(filterAcceptsRow(i, sourceIndex))
          {
            return true;
          }
        }

        const CEosScriptModelItem* pItem = pSourceModel->GetItem(sourceIndex);
        if (nullptr == pItem)
        {
          return false;
        }

        QString sData = pItem->Data(eos_item::c_iColumnName, Qt::DisplayRole).toString();
        return sData.contains(filterRegExp());
      }
    }
  }

  return QSortFilterProxyModel::filterAcceptsRow(iSourceRow, sourceParent);
}

//----------------------------------------------------------------------------------------
//
void CEosSortFilterProxyModel::sort(int, Qt::SortOrder)
{
  // do nothing, as we do not want to sort anything
}
