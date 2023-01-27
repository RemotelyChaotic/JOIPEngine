#include "CommandUpdateEosCommand.h"
#include "Editor/EditorCommandIds.h"
#include "Editor/Script/EosScriptModel.h"

//----------------------------------------------------------------------------------------
//
CCommandUpdateEosCommand::CCommandUpdateEosCommand(
    QPointer<CEosScriptModel> pModel,
    QPointer<CEosScriptEditorView> pView,
    const SItemIndexPath& path,
    const tInstructionMapValue& argsOld,
    const tInstructionMapValue& argsNew,
    bool bInvalidate,
    QUndoCommand* pParent) :
  QUndoCommand(QString("Changed parameters of %1").arg(path.m_sName), pParent),
  m_pModel(pModel),
  m_pView(pView),
  m_pathCurrent(path),
  m_argsOld(argsOld),
  m_argsNew(argsNew),
  m_bInvalidate(bInvalidate)
{
}
CCommandUpdateEosCommand::~CCommandUpdateEosCommand()
{
}

//----------------------------------------------------------------------------------------
//
void CCommandUpdateEosCommand::undo()
{
  DoWithArgs(m_argsOld);
}

//----------------------------------------------------------------------------------------
//
void CCommandUpdateEosCommand::redo()
{
  DoWithArgs(m_argsNew);
}

//----------------------------------------------------------------------------------------
//
int CCommandUpdateEosCommand::id() const
{
  return EEditorCommandId::eUpdateEosCommand;
}

//----------------------------------------------------------------------------------------
//
bool CCommandUpdateEosCommand::mergeWith(const QUndoCommand* pOther)
{
  if (nullptr == pOther || id() != pOther->id())
  {
    return false;
  }

  const CCommandUpdateEosCommand* pOtherCasted = dynamic_cast<const CCommandUpdateEosCommand*>(pOther);
  if (nullptr == pOtherCasted ||
      pOtherCasted->m_pathCurrent != m_pathCurrent) { return false; }

  m_argsNew = pOtherCasted->m_argsNew;
  m_bInvalidate |= pOtherCasted->m_bInvalidate;
  return true;
}

//----------------------------------------------------------------------------------------
//
void CCommandUpdateEosCommand::DoWithArgs(const tInstructionMapValue& args)
{
  if (nullptr != m_pModel)
  {
    QModelIndex currentMapped = m_pModel->GetIndex(m_pathCurrent);
    if (currentMapped.isValid())
    {
      CEosScriptModelItem* pItem = m_pModel->GetItem(currentMapped);
      *pItem->Arguments() = args;
      m_pModel->Update(currentMapped);
      if (m_bInvalidate)
      {
        m_pModel->Invalidate(currentMapped);
        if (nullptr != m_pView)
        {
          m_pView->ExpandAll();
        }
      }
    }
  }
}
