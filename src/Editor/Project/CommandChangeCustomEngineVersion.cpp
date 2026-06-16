#include "CommandChangeCustomEngineVersion.h"
#include "Editor/EditorCommandIds.h"
#include "Editor/EditorWidgetTypes.h"

#include <QVariant>

CCommandChangeCustomEngineVersion::CCommandChangeCustomEngineVersion(QPointer<QCheckBox> pCheckBox,
                                                                     QPointer<QLabel> pWarningLabel,
                                                                     QList<QPointer<QWidget>> vpWidgetsToEnable,
                                                                     std::function<void(void)> fnOnUndoRedo,
                                                                     QUndoCommand* pParent) :
  QUndoCommand(QString("Custon Engine Version -> ") + (pCheckBox->isChecked() ? "true" : "false"), pParent),
  m_vpWidgetsToEnable(vpWidgetsToEnable),
  m_pCheckBox(pCheckBox),
  m_pWarningLabel(pWarningLabel),
  m_fnOnUndoRedo(fnOnUndoRedo),
  m_bOldValue(pCheckBox->property(editor::c_sPropertyOldValue).toBool()),
  m_bNewValue(pCheckBox->isChecked())
{}
CCommandChangeCustomEngineVersion::~CCommandChangeCustomEngineVersion() = default;

//----------------------------------------------------------------------------------------
//
void CCommandChangeCustomEngineVersion::undo()
{
  SetNewValue(m_bOldValue);
}

//----------------------------------------------------------------------------------------
//
void CCommandChangeCustomEngineVersion::redo()
{
  SetNewValue(m_bNewValue);
}

//----------------------------------------------------------------------------------------
//
int CCommandChangeCustomEngineVersion::id() const
{
  return EEditorCommandId::eManualEngineVersion;
}

//----------------------------------------------------------------------------------------
//
bool CCommandChangeCustomEngineVersion::mergeWith(const QUndoCommand* pOther)
{
  if (nullptr == pOther || id() != pOther->id())
  {
    return false;
  }

  const CCommandChangeCustomEngineVersion* pOtherCasted =
      dynamic_cast<const CCommandChangeCustomEngineVersion*>(pOther);
  if (nullptr == pOtherCasted) { return false; }

  m_bNewValue = pOtherCasted->m_bNewValue;
  setText(QString("Custon Engine Version -> ") + (m_bNewValue ? "true" : "false"));
  return true;

}

//----------------------------------------------------------------------------------------
//
void CCommandChangeCustomEngineVersion::SetNewValue(bool bValue)
{
  for (auto pW : qAsConst(m_vpWidgetsToEnable))
  {
    if (nullptr != pW)
    {
      pW->setEnabled(bValue);
    }
  }
  if (nullptr != m_pCheckBox)
  {
    m_pCheckBox->blockSignals(true);
    m_pCheckBox->setProperty(editor::c_sPropertyOldValue, bValue);
    m_pCheckBox->setChecked(bValue);
    m_pCheckBox->blockSignals(false);
  }
  if (nullptr != m_pWarningLabel)
  {
    m_pWarningLabel->setVisible(bValue);
  }
  if (nullptr != m_fnOnUndoRedo)
  {
    m_fnOnUndoRedo();
  }
}
