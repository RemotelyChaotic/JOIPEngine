#include "CommandChangeOpenedSequence.h"
#include "Application.h"
#include "TimelineWidget.h"

#include "Editor/EditorEditableFileModel.h"
#include "Editor/EditorCommandIds.h"

#include "Systems/DatabaseManager.h"

#include <QJsonDocument>

CCommandChangeOpenedSequence::CCommandChangeOpenedSequence(QPointer<QComboBox> pResourcesComboBox,
                                                           QPointer<CTimelineWidget> pScriptDisplayWidget,
                                                           QPointer<QWidget> pGuard,
                                                           const std::function<void(qint32)>& fnReloadEditor,
                                                           bool* pbChangingIndexFlag,
                                                           QString* psLastCachedScript,
                                                           const QString& sOldSequence,
                                                           const QString& sNewSequence,
                                                           QUndoCommand* pParent) :
  QUndoCommand("Opened sequence: " + sNewSequence, pParent),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_pResourcesComboBox(pResourcesComboBox),
  m_pTimelineWidget(pScriptDisplayWidget),
  m_pGuard(pGuard),
  m_fnReloadEditor(fnReloadEditor),
  m_pbChangingIndexFlag(pbChangingIndexFlag),
  m_psLastCachedScript(psLastCachedScript),
  m_sOldSequence(sOldSequence),
  m_sNewSequence(sNewSequence)
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
CCommandChangeOpenedSequence::~CCommandChangeOpenedSequence()
{}

//----------------------------------------------------------------------------------------
//
void CCommandChangeOpenedSequence::undo()
{
  DoUndoRedo(m_sOldSequence);
}

//----------------------------------------------------------------------------------------
//
void CCommandChangeOpenedSequence::redo()
{
  DoUndoRedo(m_sNewSequence);
}

//----------------------------------------------------------------------------------------
//
int CCommandChangeOpenedSequence::id() const
{
  return EEditorCommandId::eChangeOpenedSequence;
}

//----------------------------------------------------------------------------------------
//
bool CCommandChangeOpenedSequence::mergeWith(const QUndoCommand* pOther)
{
  if (nullptr == pOther || id() != pOther->id())
  {
    return false;
  }

  const CCommandChangeOpenedSequence* pOtherCasted = dynamic_cast<const CCommandChangeOpenedSequence*>(pOther);
  if (nullptr == pOtherCasted) { return false; }

  m_sNewSequence = pOtherCasted->m_sNewSequence;

  setText("Opened sequence: " + m_sNewSequence);
  return true;
}

//----------------------------------------------------------------------------------------
//
void CCommandChangeOpenedSequence::DoUndoRedo(const QString& sSequenceNext)
{
  if (!m_pGuard.isNull() && !m_pEditorModel.isNull() &&
      nullptr != m_pbChangingIndexFlag && nullptr != m_psLastCachedScript)
  {
    *m_pbChangingIndexFlag = true;

    // save old contents
    auto pScriptItem = m_pEditorModel->CachedFile(*m_psLastCachedScript);
    if (nullptr != pScriptItem)
    {
      tspSequence spSequence = m_pTimelineWidget->Sequence();
      QJsonDocument doc(spSequence->ToJsonObject());
      pScriptItem->m_data = doc.toJson(QJsonDocument::Indented);
      m_pEditorModel->SetSceneScriptModifiedFlag(pScriptItem->m_sId, pScriptItem->m_bChanged);
    }

    if (nullptr != m_fnReloadEditor)
    {
      m_fnReloadEditor(m_pEditorModel->FileIndex(sSequenceNext));
    }

    *m_pbChangingIndexFlag = false;
  }
}
