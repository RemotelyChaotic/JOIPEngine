#include "CommandChangeFont.h"
#include "Editor/EditorCommandIds.h"
#include "Editor/EditorWidgetTypes.h"

CCommandChangeFont::CCommandChangeFont(QPointer<QFontComboBox> pFontComboBox,
                                       QUndoCommand* pParent) :
  QUndoCommand("Changed Project font -> " + m_pFontComboBox->currentFont().family(), pParent),
  m_pFontComboBox(pFontComboBox),
  m_sOriginalValue(m_pFontComboBox->property(editor::c_sPropertyOldValue).toString()),
  m_sNewValue(m_pFontComboBox->currentFont().family())
{
}
CCommandChangeFont::~CCommandChangeFont()
{}

//----------------------------------------------------------------------------------------
//
void CCommandChangeFont::undo()
{
  m_pFontComboBox->blockSignals(true);
  m_pFontComboBox->setProperty(editor::c_sPropertyOldValue, m_sOriginalValue);
  m_pFontComboBox->setFont(m_sOriginalValue);
  m_pFontComboBox->blockSignals(false);
}

//----------------------------------------------------------------------------------------
//
void CCommandChangeFont::redo()
{
  m_pFontComboBox->blockSignals(true);
  m_pFontComboBox->setProperty(editor::c_sPropertyOldValue, m_sNewValue);
  m_pFontComboBox->setFont(m_sNewValue);
  m_pFontComboBox->blockSignals(false);
}

//----------------------------------------------------------------------------------------
//
int CCommandChangeFont::id() const
{
  return EEditorCommandId::eChangeFont;
}

//----------------------------------------------------------------------------------------
//
bool CCommandChangeFont::mergeWith(const QUndoCommand* pOther)
{
  if (nullptr == pOther || id() != pOther->id())
  {
    return false;
  }

  const CCommandChangeFont* pOtherCasted = dynamic_cast<const CCommandChangeFont*>(pOther);
  if (nullptr == pOtherCasted) { return false; }

  m_sNewValue = pOtherCasted->m_sNewValue;
  setText("Changed Project font -> " + m_sNewValue);
  return true;
}
