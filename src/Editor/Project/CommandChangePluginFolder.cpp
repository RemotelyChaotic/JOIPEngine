#include "CommandChangePluginFolder.h"
#include "Editor/EditorCommandIds.h"
#include "Editor/EditorWidgetTypes.h"

CCommandChangePluginFolder::CCommandChangePluginFolder(QPointer<QLineEdit> pLineEdit,
                                       const std::function<void(void)>& fnOnUndoRedo,
                                       QUndoCommand* pParent) :
  QUndoCommand("Changed Plugin folder -> " + pLineEdit->text(), pParent),
  m_pLineEdit(pLineEdit),
  m_fnOnUndoRedo(fnOnUndoRedo),
  m_sOriginalValue(m_pLineEdit->property(editor::c_sPropertyOldValue).toString()),
  m_sNewValue(m_pLineEdit->text())
{
}
CCommandChangePluginFolder::~CCommandChangePluginFolder()
{}

//----------------------------------------------------------------------------------------
//
void CCommandChangePluginFolder::undo()
{
  m_pLineEdit->blockSignals(true);
  m_pLineEdit->setProperty(editor::c_sPropertyOldValue, m_sOriginalValue);
  m_pLineEdit->setText(m_sOriginalValue);
  m_pLineEdit->blockSignals(false);
  if (nullptr != m_fnOnUndoRedo)
  {
    m_fnOnUndoRedo();
  }
}

//----------------------------------------------------------------------------------------
//
void CCommandChangePluginFolder::redo()
{
  m_pLineEdit->blockSignals(true);
  m_pLineEdit->setProperty(editor::c_sPropertyOldValue, m_sNewValue);
  m_pLineEdit->setText(m_sNewValue);
  m_pLineEdit->blockSignals(false);
  if (nullptr != m_fnOnUndoRedo)
  {
    m_fnOnUndoRedo();
  }
}

//----------------------------------------------------------------------------------------
//
int CCommandChangePluginFolder::id() const
{
  return EEditorCommandId::eChangePluginPath;
}

//----------------------------------------------------------------------------------------
//
bool CCommandChangePluginFolder::mergeWith(const QUndoCommand* pOther)
{
  if (nullptr == pOther || id() != pOther->id())
  {
    return false;
  }

  const CCommandChangePluginFolder* pOtherCasted = dynamic_cast<const CCommandChangePluginFolder*>(pOther);
  if (nullptr == pOtherCasted) { return false; }

  m_sNewValue = pOtherCasted->m_sNewValue;
  setText("Changed Plugin folder -> " + m_sNewValue);
  return true;
}
