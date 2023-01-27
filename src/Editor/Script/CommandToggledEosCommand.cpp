#include "CommandToggledEosCommand.h"
#include "Editor/EditorCommandIds.h"
#include "Editor/Script/EosScriptModel.h"

//----------------------------------------------------------------------------------------
//
CCommandToggledEosCommand::CCommandToggledEosCommand(
    QPointer<CEosScriptModel> pModel,
    const SItemIndexPath& path,
    bool bOldValue,
    bool bNewValue,
    QUndoCommand* pParent) :
  QUndoCommand(QString("Toggled %1 -> %2").arg(path.m_sName).arg(bNewValue ? "enabled" : "disabled"), pParent),
  m_pModel(pModel),
  m_path(path),
  m_bOldValue(bOldValue),
  m_bNewValue(bNewValue)
{
}
CCommandToggledEosCommand::~CCommandToggledEosCommand()
{

}

//----------------------------------------------------------------------------------------
//
void CCommandToggledEosCommand::undo()
{
  if (nullptr != m_pModel)
  {
    QModelIndex idx = m_pModel->GetIndex(m_path);
    CEosScriptModelItem* pItem = m_pModel->GetItem(idx);
    if (nullptr != pItem)
    {
      pItem->SetChecked(m_bOldValue);
      emit m_pModel->Update(idx);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CCommandToggledEosCommand::redo()
{
  if (nullptr != m_pModel)
  {
    QModelIndex idx = m_pModel->GetIndex(m_path);
    CEosScriptModelItem* pItem = m_pModel->GetItem(idx);
    if (nullptr != pItem)
    {
      pItem->SetChecked(m_bNewValue);
      emit m_pModel->Update(idx);
    }
  }
}

//----------------------------------------------------------------------------------------
//
int CCommandToggledEosCommand::id() const
{
  return EEditorCommandId::eToggleEosCommand;
}

//----------------------------------------------------------------------------------------
//
bool CCommandToggledEosCommand::mergeWith(const QUndoCommand* pOther)
{
  if (nullptr == pOther || id() != pOther->id())
  {
    return false;
  }

  const CCommandToggledEosCommand* pOtherCasted = dynamic_cast<const CCommandToggledEosCommand*>(pOther);
  if (nullptr == pOtherCasted ||
      pOtherCasted->m_path != m_path) { return false; }

  m_bNewValue = pOtherCasted->m_bNewValue;
  setText(QString("Toggled %1 -> %2").arg(m_path.m_sName).arg(m_bNewValue ? "enabled" : "disabled"));
  return true;
}
