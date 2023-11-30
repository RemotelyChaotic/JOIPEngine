#include "CommandChangeLayout.h"
#include "Editor/EditorCommandIds.h"
#include "Editor/EditorWidgetTypes.h"
#include "Systems/Project.h"

CCommandChangeLayout::CCommandChangeLayout(QPointer<QComboBox> pComboBox,
                                           const tspProject& spCurrentProject,
                                           const std::function<void(void)>& fnOnUndoRedo,
                                           QUndoCommand* pParent) :
  QUndoCommand("Changed Project Default Layout -> " + pComboBox->currentText(), pParent),
  m_pComboBox(pComboBox),
  m_fnOnUndoRedo(fnOnUndoRedo),
  m_spCurrentProject(spCurrentProject),
  m_sOriginalValue(pComboBox->property(editor::c_sPropertyOldValue).toString()),
  m_sNewValue(m_pComboBox->currentData().toString())
{
}

CCommandChangeLayout::~CCommandChangeLayout()
{}

//----------------------------------------------------------------------------------------
//
void CCommandChangeLayout::undo()
{
  qint32 iIdx = m_pComboBox->findData(m_sOriginalValue);
  m_pComboBox->blockSignals(true);
  m_pComboBox->setProperty(editor::c_sPropertyOldValue, m_sOriginalValue);
  m_pComboBox->setCurrentIndex(iIdx);
  m_pComboBox->blockSignals(false);
  if (nullptr != m_spCurrentProject)
  {
    QWriteLocker locker(&m_spCurrentProject->m_rwLock);
    m_spCurrentProject->m_sPlayerLayout = m_pComboBox->itemData(iIdx).toString();
  }
  if (nullptr != m_fnOnUndoRedo)
  {
    m_fnOnUndoRedo();
  }
}

//----------------------------------------------------------------------------------------
//
void CCommandChangeLayout::redo()
{
  qint32 iIdx = m_pComboBox->findData(m_sNewValue);
  m_pComboBox->blockSignals(true);
  m_pComboBox->setProperty(editor::c_sPropertyOldValue, m_sNewValue);
  m_pComboBox->setCurrentIndex(iIdx);
  m_pComboBox->blockSignals(false);
  if (nullptr != m_spCurrentProject)
  {
    QWriteLocker locker(&m_spCurrentProject->m_rwLock);
    m_spCurrentProject->m_sPlayerLayout = m_pComboBox->itemData(iIdx).toString();
  }
  if (nullptr != m_fnOnUndoRedo)
  {
    m_fnOnUndoRedo();
  }
}

//----------------------------------------------------------------------------------------
//
int CCommandChangeLayout::id() const
{
  return EEditorCommandId::eChangeDefaultLayout;
}

//----------------------------------------------------------------------------------------
//
bool CCommandChangeLayout::mergeWith(const QUndoCommand* pOther)
{
  if (nullptr == pOther || id() != pOther->id())
  {
    return false;
  }

  const CCommandChangeLayout* pOtherCasted = dynamic_cast<const CCommandChangeLayout*>(pOther);
  if (nullptr == pOtherCasted) { return false; }

  m_sNewValue = pOtherCasted->m_sNewValue;
  qint32 iIdx = m_pComboBox->findData(m_sNewValue);
  if (-1 != iIdx)
  {
    setText("Changed Project Default Layout -> " + m_pComboBox->itemText(iIdx));
  }
  else
  {
    setText("Changed Project Default Layout -> " + m_sNewValue);
  }
  return true;
}
