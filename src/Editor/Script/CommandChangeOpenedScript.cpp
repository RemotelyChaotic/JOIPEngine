#include "CommandChangeOpenedScript.h"
#include "Application.h"
#include "CodeDisplayWidget.h"
#include "Editor/EditorEditableFileModel.h"
#include "Editor/EditorCommandIds.h"
#include "Systems/DatabaseManager.h"

CCommandChangeOpenedScript::CCommandChangeOpenedScript(QPointer<QComboBox> pResourcesComboBox,
                                                       QPointer<CCodeDisplayWidget> pScriptDisplayWidget,
                                                       QPointer<QWidget> pGuard,
                                                       const std::function<void(qint32)>& fnReloadEditor,
                                                       bool* pbChangingIndexFlag,
                                                       QString* psLastCachedScript,
                                                       const QString& sOldScript,
                                                       const QString& sNewScript,
                                                       QUndoCommand* pParent) :
  QUndoCommand("Opened script: " + sNewScript, pParent),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_pResourcesComboBox(pResourcesComboBox),
  m_pScriptDisplayWidget(pScriptDisplayWidget),
  m_pEditorModel(dynamic_cast<CEditorEditableFileModel*>(pResourcesComboBox->model())),
  m_pGuard(pGuard),
  m_fnReloadEditor(fnReloadEditor),
  m_pbChangingIndexFlag(pbChangingIndexFlag),
  m_psLastCachedScript(psLastCachedScript),
  m_sOldScript(sOldScript),
  m_sNewScript(sNewScript)
{
}
CCommandChangeOpenedScript::~CCommandChangeOpenedScript()
{}

//----------------------------------------------------------------------------------------
//
void CCommandChangeOpenedScript::undo()
{
  DoUndoRedo(m_sOldScript);
}

//----------------------------------------------------------------------------------------
//
void CCommandChangeOpenedScript::redo()
{
  DoUndoRedo(m_sNewScript);
}

//----------------------------------------------------------------------------------------
//
int CCommandChangeOpenedScript::id() const
{
  return EEditorCommandId::eChangeOpenedScript;
}

//----------------------------------------------------------------------------------------
//
bool CCommandChangeOpenedScript::mergeWith(const QUndoCommand* pOther)
{
  if (nullptr == pOther || id() != pOther->id())
  {
    return false;
  }

  const CCommandChangeOpenedScript* pOtherCasted = dynamic_cast<const CCommandChangeOpenedScript*>(pOther);
  if (nullptr == pOtherCasted) { return false; }

  m_sNewScript = pOtherCasted->m_sNewScript;

  setText("Opened script: " + m_sNewScript);
  return true;
}

//----------------------------------------------------------------------------------------
//
void CCommandChangeOpenedScript::DoUndoRedo(const QString& sScriptNext)
{
  if (!m_pGuard.isNull() && !m_pEditorModel.isNull() &&
      nullptr != m_pbChangingIndexFlag && nullptr != m_psLastCachedScript)
  {
    *m_pbChangingIndexFlag = true;

    // save old contents
    auto pScriptItem = m_pEditorModel->CachedFile(*m_psLastCachedScript);
    if (nullptr != pScriptItem)
    {
      pScriptItem->m_data = m_pScriptDisplayWidget->GetCurrentText().toUtf8();
      m_pEditorModel->SetSceneScriptModifiedFlag(pScriptItem->m_sId, pScriptItem->m_bChanged);
    }

    if (nullptr != m_fnReloadEditor)
    {
      m_fnReloadEditor(m_pEditorModel->FileIndex(sScriptNext));
    }

    *m_pbChangingIndexFlag = false;
  }
}
