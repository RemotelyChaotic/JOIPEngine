#include "CommandRemoveEosCommand.h"
#include "Editor/EditorCommandIds.h"
#include "Editor/Script/EosScriptModel.h"

//----------------------------------------------------------------------------------------
//
CCommandRemoveEosCommand::CCommandRemoveEosCommand(
    QPointer<CEosScriptModel> pModel,
    const SItemIndexPath& path,
    QUndoCommand* pParent) :
  QUndoCommand(QString("Removed %1").arg(path.m_sName), pParent),
  m_pModel(pModel),
  m_pathParent(),
  m_pathNodeParent(),
  m_pathRemoved(path),
  m_iIndex(-1),
  m_iChildIndex(-1),
  m_parentType(EosScriptModelItem::eRoot),
  m_sType(),
  m_sChildGroup(),
  m_args()
{
}
CCommandRemoveEosCommand::~CCommandRemoveEosCommand()
{

}

//----------------------------------------------------------------------------------------
//
void CCommandRemoveEosCommand::undo()
{
  if (nullptr != m_pModel)
  {
    CEosScriptModelItem* pParent = m_pModel->GetItem(m_pathNodeParent);
    m_pModel->InsertInstructionAtImpl(
          m_pModel->GetIndex(m_pathParent), m_iIndex, m_iChildIndex, m_sChildGroup,
          m_sType, m_parentType._to_integral(), m_args, pParent->Node());
  }
}

//----------------------------------------------------------------------------------------
//
void CCommandRemoveEosCommand::redo()
{
  if (nullptr != m_pModel)
  {
    CEosScriptModelItem* pItem = m_pModel->GetItem(m_pathRemoved);
    if (nullptr == pItem) { assert(false); return; }
    auto spNode =  pItem->Node();
    if (nullptr == spNode) { assert(false); return; }

    QModelIndex idx = m_pModel->GetIndex(m_pathRemoved);
    m_sType = spNode->m_sName;
    m_args = *pItem->Arguments();

    QModelIndex parent = m_pModel->parent(idx);
    CEosScriptModelItem* pParent = pItem->Parent();
    std::shared_ptr<CJsonInstructionNode> spParentNewNode = pParent->Node();
    if (nullptr == pParent || nullptr == spParentNewNode) { assert(false); return; }

    m_iChildIndex = pParent->ChildIndex(pItem);
    m_iIndex = std::find(spParentNewNode->m_spChildren.begin(),
                         spParentNewNode->m_spChildren.end(), spNode) -
        spParentNewNode->m_spChildren.begin();
    m_parentType = pParent->Type();

    switch(m_parentType)
    {
      case EosScriptModelItem::eInstructionChild:
      {
        m_sChildGroup = pParent->Data(eos_item::c_iColumnName, Qt::DisplayRole).toString();
        m_pModel->GetIndexPath(m_pModel->parent(parent), -1, &m_pathNodeParent);
      } break;
      default:
      {
        m_pModel->GetIndexPath(parent, -1, &m_pathNodeParent);
      } break;
    }

    m_pModel->GetIndexPath(parent, -1, &m_pathParent);
    m_pModel->RemoveInstructionImpl(idx);
  }
}

//----------------------------------------------------------------------------------------
//
int CCommandRemoveEosCommand::id() const
{
  return EEditorCommandId::eRemoveEosCommand;
}

//----------------------------------------------------------------------------------------
//
bool CCommandRemoveEosCommand::mergeWith(const QUndoCommand*)
{
  return false;
}
