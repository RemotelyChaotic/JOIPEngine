#include "CommandChangeToyCmd.h"
#include "Editor/EditorCommandIds.h"
#include "Editor/EditorWidgetTypes.h"
#include "Systems/Database/Project.h"

CCommandChangeToyCmd::CCommandChangeToyCmd(QPointer<QComboBox> pComboBox,
                                           const tspProject& spCurrentProject,
                                           const std::function<void(void)>& fnOnUndoRedo,
                                           QUndoCommand* pParent) :
    QUndoCommand("Changed Metronome Toy Command -> " + pComboBox->currentText(), pParent),
    m_pComboBox(pComboBox),
    m_fnOnUndoRedo(fnOnUndoRedo),
    m_spCurrentProject(spCurrentProject),
    m_iOriginalValue(pComboBox->property(editor::c_sPropertyOldValue).toInt()),
    m_iNewValue(m_pComboBox->currentData().toInt())
{
}
CCommandChangeToyCmd::~CCommandChangeToyCmd() = default;

//----------------------------------------------------------------------------------------
//
void CCommandChangeToyCmd::undo()
{
  qint32 iIdx = m_pComboBox->findData(m_iOriginalValue);
  m_pComboBox->blockSignals(true);
  m_pComboBox->setProperty(editor::c_sPropertyOldValue, m_iOriginalValue);
  m_pComboBox->setCurrentIndex(iIdx);
  m_pComboBox->blockSignals(false);
  if (nullptr != m_spCurrentProject)
  {
    QWriteLocker locker(&m_spCurrentProject->m_rwLock);
    m_spCurrentProject->m_metCmdMode = m_pComboBox->itemData(iIdx).toInt();
  }
  if (nullptr != m_fnOnUndoRedo)
  {
    m_fnOnUndoRedo();
  }
}

//----------------------------------------------------------------------------------------
//
void CCommandChangeToyCmd::redo()
{
  qint32 iIdx = m_pComboBox->findData(m_iNewValue);
  m_pComboBox->blockSignals(true);
  m_pComboBox->setProperty(editor::c_sPropertyOldValue, m_iNewValue);
  m_pComboBox->setCurrentIndex(iIdx);
  m_pComboBox->blockSignals(false);
  if (nullptr != m_spCurrentProject)
  {
    QWriteLocker locker(&m_spCurrentProject->m_rwLock);
    m_spCurrentProject->m_metCmdMode = m_pComboBox->itemData(iIdx).toInt();
  }
  if (nullptr != m_fnOnUndoRedo)
  {
    m_fnOnUndoRedo();
  }
}

//----------------------------------------------------------------------------------------
//
int CCommandChangeToyCmd::id() const
{
  return EEditorCommandId::eChangeMetronomeToyCommand;
}

//----------------------------------------------------------------------------------------
//
bool CCommandChangeToyCmd::mergeWith(const QUndoCommand* pOther)
{
  if (nullptr == pOther || id() != pOther->id())
  {
    return false;
  }

  const CCommandChangeToyCmd* pOtherCasted = dynamic_cast<const CCommandChangeToyCmd*>(pOther);
  if (nullptr == pOtherCasted) { return false; }

  m_iNewValue = pOtherCasted->m_iNewValue;
  qint32 iIdx = m_pComboBox->findData(m_iNewValue);
  if (-1 != iIdx)
  {
    setText("Changed Metronome Toy Command -> " + m_pComboBox->itemText(iIdx));
  }
  else
  {
    setText("Changed Metronome Toy Command -> " + QString::number(m_iNewValue));
  }
  return true;
}
