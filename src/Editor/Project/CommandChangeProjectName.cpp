#include "CommandChangeProjectName.h"
#include "Editor/EditorCommandIds.h"
#include "Editor/EditorModel.h"
#include "Editor/EditorWidgetTypes.h"

CCommandChangeProjectName::CCommandChangeProjectName(QPointer<QLineEdit> pNameLineEdit,
                          QPointer<CEditorModel> pEditorModel,
                          QUndoCommand* pParent) :
  QUndoCommand("Project renamed -> " + pNameLineEdit->text(), pParent),
  m_pNameLineEdit(pNameLineEdit),
  m_pEditorModel(pEditorModel),
  m_sOldName(pNameLineEdit->property(editor::c_sPropertyOldValue).toString()),
  m_sNewName(pNameLineEdit->text())
{

}
CCommandChangeProjectName::~CCommandChangeProjectName()
{

}

//----------------------------------------------------------------------------------------
//
void CCommandChangeProjectName::undo()
{
  m_sOldName = m_pEditorModel->RenameProject(m_sOldName);
  m_pNameLineEdit->blockSignals(true);
  m_pNameLineEdit->setText(m_sOldName);
  m_pNameLineEdit->blockSignals(false);
}

//----------------------------------------------------------------------------------------
//
void CCommandChangeProjectName::redo()
{
  m_sNewName = m_pEditorModel->RenameProject(m_sNewName);
  m_pNameLineEdit->blockSignals(true);
  m_pNameLineEdit->setText(m_sNewName);
  m_pNameLineEdit->blockSignals(false);
}

//----------------------------------------------------------------------------------------
//
int CCommandChangeProjectName::id() const
{
  return EEditorCommandId::eChangeProjectName;
}

//----------------------------------------------------------------------------------------
//
bool CCommandChangeProjectName::mergeWith(const QUndoCommand* pOther)
{
  if (nullptr == pOther || id() != pOther->id())
  {
    return false;
  }

  const CCommandChangeProjectName* pOtherCasted = dynamic_cast<const CCommandChangeProjectName*>(pOther);
  if (nullptr == pOtherCasted) { return false; }

  m_sNewName = pOtherCasted->m_sNewName;

  setText("Project renamed -> " + m_sNewName);
  return true;
}
