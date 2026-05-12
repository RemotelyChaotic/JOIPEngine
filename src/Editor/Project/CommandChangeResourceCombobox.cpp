#include "CommandChangeResourceComboBox.h"
#include "Editor/EditorCommandIds.h"
#include "Editor/EditorWidgetTypes.h"
#include "Systems/Database/Project.h"

CCommandChangeResourceComboBox::CCommandChangeResourceComboBox(
    QPointer<QComboBox> pComboBox,
    const tspProject& spCurrentProject,
    const std::function<void(void)>& fnOnUndoRedo,
    const QString& sText,
    QUndoCommand* pParent) :
  QUndoCommand(sText, pParent),
  m_pComboBox(pComboBox),
  m_fnOnUndoRedo(fnOnUndoRedo),
  m_spCurrentProject(spCurrentProject),
  m_sOriginalValue(pComboBox->property(editor::c_sPropertyOldValue).toString()),
  m_sNewValue(m_pComboBox->currentData().toString())
{
}

CCommandChangeResourceComboBox::~CCommandChangeResourceComboBox()
{}

//----------------------------------------------------------------------------------------
//
void CCommandChangeResourceComboBox::undo()
{
  qint32 iIdx = m_pComboBox->findData(m_sOriginalValue);
  m_pComboBox->blockSignals(true);
  m_pComboBox->setProperty(editor::c_sPropertyOldValue, m_sOriginalValue);
  m_pComboBox->setCurrentIndex(iIdx);
  m_pComboBox->blockSignals(false);
  if (nullptr != m_spCurrentProject)
  {
    SetNewValue(m_pComboBox->itemData(iIdx).toString());
  }
  if (nullptr != m_fnOnUndoRedo)
  {
    m_fnOnUndoRedo();
  }
}

//----------------------------------------------------------------------------------------
//
void CCommandChangeResourceComboBox::redo()
{
  qint32 iIdx = m_pComboBox->findData(m_sNewValue);
  m_pComboBox->blockSignals(true);
  m_pComboBox->setProperty(editor::c_sPropertyOldValue, m_sNewValue);
  m_pComboBox->setCurrentIndex(iIdx);
  m_pComboBox->blockSignals(false);
  if (nullptr != m_spCurrentProject)
  {
    SetNewValue(m_pComboBox->itemData(iIdx).toString());
  }
  if (nullptr != m_fnOnUndoRedo)
  {
    m_fnOnUndoRedo();
  }
}

//----------------------------------------------------------------------------------------
//
int CCommandChangeResourceComboBox::id() const
{
  return -1;
}

//----------------------------------------------------------------------------------------
//
bool CCommandChangeResourceComboBox::mergeWith(const QUndoCommand* pOther)
{
  if (nullptr == pOther || id() != pOther->id())
  {
    return false;
  }

  const CCommandChangeResourceComboBox* pOtherCasted =
      dynamic_cast<const CCommandChangeResourceComboBox*>(pOther);
  if (nullptr == pOtherCasted) { return false; }

  return true;
}

//----------------------------------------------------------------------------------------
//
CCommandChangeLayout::CCommandChangeLayout(QPointer<QComboBox> pComboBox,
                                           const tspProject& spCurrentProject,
                                           const std::function<void(void)>& fnOnUndoRedo,
                                           QUndoCommand* pParent) :
  CCommandChangeResourceComboBox(pComboBox, spCurrentProject, fnOnUndoRedo,
                                 "Changed Project Default Layout -> " + pComboBox->currentText(),
                                 pParent)
{
}
CCommandChangeLayout::~CCommandChangeLayout()
{}

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
  if (!CCommandChangeResourceComboBox::mergeWith(pOther))
  {
    return false;
  }

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

//----------------------------------------------------------------------------------------
//
void CCommandChangeLayout::SetNewValue(const QString& sValue)
{
  QWriteLocker locker(&m_spCurrentProject->m_rwLock);
  m_spCurrentProject->m_sPlayerLayout = sValue;
}

//----------------------------------------------------------------------------------------
//
CCommandChangePreloadScript::CCommandChangePreloadScript(
    QPointer<QComboBox> pComboBox,
    const tspProject& spCurrentProject,
    const std::function<void(void)>& fnOnUndoRedo,
    QUndoCommand* pParent) :
    CCommandChangeResourceComboBox(pComboBox, spCurrentProject, fnOnUndoRedo,
                                   "Changed Project Preload Script -> " + pComboBox->currentText(),
                                   pParent)
{
}
CCommandChangePreloadScript::~CCommandChangePreloadScript()
{}


//----------------------------------------------------------------------------------------
//
int CCommandChangePreloadScript::id() const
{
  return EEditorCommandId::eChangePreloadScript;
}

//----------------------------------------------------------------------------------------
//
bool CCommandChangePreloadScript::mergeWith(const QUndoCommand* pOther)
{
  if (!CCommandChangeResourceComboBox::mergeWith(pOther))
  {
    return false;
  }

  qint32 iIdx = m_pComboBox->findData(m_sNewValue);
  if (-1 != iIdx)
  {
    setText("Changed Project Preload Script -> " + m_pComboBox->itemText(iIdx));
  }
  else
  {
    setText("Changed Project Preload Script -> " + m_sNewValue);
  }
  return true;
}

//----------------------------------------------------------------------------------------
//
void CCommandChangePreloadScript::SetNewValue(const QString& sValue)
{
  QWriteLocker locker(&m_spCurrentProject->m_rwLock);
  m_spCurrentProject->m_sPreLoadScript = sValue;
}
