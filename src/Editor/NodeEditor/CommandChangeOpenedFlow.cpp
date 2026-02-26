#include "CommandChangeOpenedFlow.h"
#include "Application.h"

#include "Editor/NodeEditor/NodeEditorFlowScene.h"

#include "Editor/EditorEditableFileModel.h"
#include "Editor/EditorCommandIds.h"

#include "Systems/DatabaseManager.h"

#include <QByteArray>

CCommandChangeOpenedFlow::CCommandChangeOpenedFlow(QPointer<QComboBox> pResourcesComboBox,
                                                   QPointer<CNodeEditorFlowScene> pScene,
                                                   QPointer<QWidget> pGuard,
                                                   const std::function<void(qint32)>& fnReloadEditor,
                                                   bool* pbChangingIndexFlag,
                                                   QString* psLastCachedScript,
                                                   const QString& sOldFlow,
                                                   const QString& sNewFlow,
                                                   QUndoCommand* pParent) :
    QUndoCommand("Opened flow: " + sNewFlow, pParent),
    m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
    m_pResourcesComboBox(pResourcesComboBox),
    m_pScene(pScene),
    m_pGuard(pGuard),
    m_fnReloadEditor(fnReloadEditor),
    m_pbChangingIndexFlag(pbChangingIndexFlag),
    m_psLastCachedScript(psLastCachedScript),
    m_sOldFlow(sOldFlow),
    m_sNewFlow(sNewFlow)
{
  auto pSortModel = dynamic_cast<QSortFilterProxyModel*>(pResourcesComboBox->model());
  if (nullptr != pSortModel)
  {
    m_pEditorModel = dynamic_cast<CEditorEditableFileModel*>(pSortModel->sourceModel());
  }
  else
  {
    m_pEditorModel = dynamic_cast<CEditorEditableFileModel*>(pResourcesComboBox->model());
  }
}
CCommandChangeOpenedFlow::~CCommandChangeOpenedFlow()
{}

//----------------------------------------------------------------------------------------
//
void CCommandChangeOpenedFlow::undo()
{
  DoUndoRedo(m_sOldFlow);
}

//----------------------------------------------------------------------------------------
//
void CCommandChangeOpenedFlow::redo()
{
  DoUndoRedo(m_sNewFlow);
}

//----------------------------------------------------------------------------------------
//
int CCommandChangeOpenedFlow::id() const
{
  return EEditorCommandId::eChangeOpenedFlow;
}

//----------------------------------------------------------------------------------------
//
bool CCommandChangeOpenedFlow::mergeWith(const QUndoCommand* pOther)
{
  if (nullptr == pOther || id() != pOther->id())
  {
    return false;
  }

  const CCommandChangeOpenedFlow* pOtherCasted = dynamic_cast<const CCommandChangeOpenedFlow*>(pOther);
  if (nullptr == pOtherCasted) { return false; }

  m_sNewFlow = pOtherCasted->m_sNewFlow;

  setText("Opened flow: " + m_sNewFlow);
  return true;
}

//----------------------------------------------------------------------------------------
//
void CCommandChangeOpenedFlow::DoUndoRedo(const QString& sSequenceNext)
{
  if (!m_pGuard.isNull() && !m_pEditorModel.isNull() &&
      nullptr != m_pbChangingIndexFlag && nullptr != m_psLastCachedScript)
  {
    *m_pbChangingIndexFlag = true;

    // save old contents
    auto pFlowItem = m_pEditorModel->CachedFile(*m_psLastCachedScript);
    if (nullptr != pFlowItem)
    {
      pFlowItem->m_data = m_pScene->saveToMemory();
      m_pEditorModel->SetSceneScriptModifiedFlag(pFlowItem->m_sId, pFlowItem->m_bChanged);
    }

    if (nullptr != m_fnReloadEditor)
    {
      m_fnReloadEditor(m_pEditorModel->FileIndex(sSequenceNext));
    }

    *m_pbChangingIndexFlag = false;
  }
}
