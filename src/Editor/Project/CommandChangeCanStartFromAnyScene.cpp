#include "CommandChangeCanStartFromAnyScene.h"
#include "Editor/EditorCommandIds.h"
#include "Editor/EditorWidgetTypes.h"
#include "Systems/Database/Project.h"

CCommandChangeCanStartFromAnyScene::CCommandChangeCanStartFromAnyScene(QPointer<QCheckBox> pCheckBox,
                                                                       const std::function<void(void)>& fnOnUndoRedo,
                                                                       QUndoCommand* pParent) :
  QUndoCommand("Can start from any scene -> " + QString::number(pCheckBox->isChecked()), pParent),
  m_pCanStartFromAnySceneCheckBox(pCheckBox),
  m_fnOnUndoRedo(fnOnUndoRedo),
  m_bOriginalValue(pCheckBox->property(editor::c_sPropertyOldValue).toBool()),
  m_bNewValue(pCheckBox->isChecked())
{
}
CCommandChangeCanStartFromAnyScene::~CCommandChangeCanStartFromAnyScene() = default;

//----------------------------------------------------------------------------------------
//
void CCommandChangeCanStartFromAnyScene::undo()
{
  m_pCanStartFromAnySceneCheckBox->blockSignals(true);
  m_pCanStartFromAnySceneCheckBox->setProperty(editor::c_sPropertyOldValue, m_bOriginalValue);
  m_pCanStartFromAnySceneCheckBox->setChecked(m_bOriginalValue);
  m_pCanStartFromAnySceneCheckBox->blockSignals(false);
  if (nullptr != m_fnOnUndoRedo)
  {
    m_fnOnUndoRedo();
  }
}

//----------------------------------------------------------------------------------------
//
void CCommandChangeCanStartFromAnyScene::redo()
{
  m_pCanStartFromAnySceneCheckBox->blockSignals(true);
  m_pCanStartFromAnySceneCheckBox->setProperty(editor::c_sPropertyOldValue, m_bNewValue);
  m_pCanStartFromAnySceneCheckBox->setChecked(m_bNewValue);
  m_pCanStartFromAnySceneCheckBox->blockSignals(false);
  if (nullptr != m_fnOnUndoRedo)
  {
    m_fnOnUndoRedo();
  }
}

//----------------------------------------------------------------------------------------
//
int CCommandChangeCanStartFromAnyScene::id() const
{
  return EEditorCommandId::eChangeCanStartFomAnyScene;
}

//----------------------------------------------------------------------------------------
//
bool CCommandChangeCanStartFromAnyScene::mergeWith(const QUndoCommand* pOther)
{
  if (nullptr == pOther || id() != pOther->id())
  {
    return false;
  }

  const CCommandChangeCanStartFromAnyScene* pOtherCasted = dynamic_cast<const CCommandChangeCanStartFromAnyScene*>(pOther);
  if (nullptr == pOtherCasted) { return false; }

  m_bNewValue = pOtherCasted->m_bNewValue;
  setText("Can start from any scene -> " + QString::number(m_bNewValue));
  return true;
}
