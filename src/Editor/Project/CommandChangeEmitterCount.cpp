#include "CommandChangeEmitterCount.h"
#include "Editor/EditorCommandIds.h"
#include "Editor/EditorWidgetTypes.h"

CCommandChangeEmitterCount::CCommandChangeEmitterCount(QPointer<QSpinBox> pEmitterCount,
                                                       QUndoCommand* pParent) :
  QUndoCommand("Emitters -> " + QString::number(pEmitterCount->value()), pParent),
  m_pEmitterCount(pEmitterCount),
  m_iOriginalValue(pEmitterCount->property(editor::c_sPropertyOldValue).toInt()),
  m_iNewValue(pEmitterCount->value())
{
}
CCommandChangeEmitterCount::~CCommandChangeEmitterCount()
{}

//----------------------------------------------------------------------------------------
//
void CCommandChangeEmitterCount::CCommandChangeEmitterCount::undo()
{
  m_pEmitterCount->blockSignals(true);
  m_pEmitterCount->setProperty(editor::c_sPropertyOldValue, m_iOriginalValue);
  m_pEmitterCount->setValue(m_iOriginalValue);
  m_pEmitterCount->blockSignals(false);
}

//----------------------------------------------------------------------------------------
//
void CCommandChangeEmitterCount::CCommandChangeEmitterCount::redo()
{
  m_pEmitterCount->blockSignals(true);
  m_pEmitterCount->setProperty(editor::c_sPropertyOldValue, m_iNewValue);
  m_pEmitterCount->setValue(m_iNewValue);
  m_pEmitterCount->blockSignals(false);
}

//----------------------------------------------------------------------------------------
//
int CCommandChangeEmitterCount::CCommandChangeEmitterCount::id() const
{
  return EEditorCommandId::eChangeEmitterCount;
}

//----------------------------------------------------------------------------------------
//
bool CCommandChangeEmitterCount::CCommandChangeEmitterCount::mergeWith(const QUndoCommand* pOther)
{
  if (nullptr == pOther || id() != pOther->id())
  {
    return false;
  }

  const CCommandChangeEmitterCount* pOtherCasted = dynamic_cast<const CCommandChangeEmitterCount*>(pOther);
  if (nullptr == pOtherCasted) { return false; }

  m_iNewValue = pOtherCasted->m_iNewValue;
  setText("Emitters -> " + QString::number(m_iNewValue));
  return true;
}
