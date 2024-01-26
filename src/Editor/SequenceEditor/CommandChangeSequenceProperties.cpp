#include "CommandChangeSequenceProperties.h"
#include "SequencePropertiesOverlay.h"

#include "Editor/EditorCommandIds.h"

CCommandChangeSequenceProperties::CCommandChangeSequenceProperties(
    QPointer<CSequencePropertiesOverlay> pOverlay,
    QPointer<QTimeEdit> pTimeEdit) :
    QUndoCommand(QString("Changed properties of: ") +
                 (nullptr != pOverlay ? pOverlay->SequenceName() : "")),
  m_pOverlay(pOverlay),
  m_pTimeEdit(pTimeEdit),
  m_sequenceOld(nullptr != pOverlay ? *pOverlay->Sequence() : SSequenceFile())
{
  // we only need the file data
  m_sequenceOld.m_vspLayers.clear();
  if (nullptr != pTimeEdit)
  {
    QTime time = pTimeEdit->time();
    m_sequenceNew.m_iLengthMili = time.minute() * 60'000 + time.second() * 1000 + time.msec();
  }
}

CCommandChangeSequenceProperties::~CCommandChangeSequenceProperties() = default;

//----------------------------------------------------------------------------------------
//
void CCommandChangeSequenceProperties::undo()
{
  DoUndoRedo(m_sequenceOld);
}

//----------------------------------------------------------------------------------------
//
void CCommandChangeSequenceProperties::redo()
{
  DoUndoRedo(m_sequenceNew);
}

//----------------------------------------------------------------------------------------
//
int CCommandChangeSequenceProperties::id() const
{
  return EEditorCommandId::eChangeSequenceProperties;
}

//----------------------------------------------------------------------------------------
//
bool CCommandChangeSequenceProperties::mergeWith(const QUndoCommand* pOther)
{
  if (nullptr == pOther || id() != pOther->id())
  {
    return false;
  }

  const CCommandChangeSequenceProperties* pOtherCasted = dynamic_cast<const CCommandChangeSequenceProperties*>(pOther);
  if (nullptr == pOtherCasted) { return false; }

  if (nullptr == m_pOverlay || nullptr == pOtherCasted->m_pOverlay) { return false; }

  m_sequenceNew = pOtherCasted->m_sequenceNew;

  setText(QString("Changed properties of: ") +
          (nullptr != m_pOverlay ? m_pOverlay->SequenceName() : ""));
  return true;
}

//----------------------------------------------------------------------------------------
//
void CCommandChangeSequenceProperties::DoUndoRedo(const SSequenceFile& file)
{
  if (nullptr != m_pOverlay && nullptr != m_pTimeEdit)
  {
    tspSequence spSeq = m_pOverlay->Sequence();
    spSeq->m_iLengthMili = file.m_iLengthMili;

    m_pOverlay->UpdateUi();
    emit m_pOverlay->SignalContentsChanged();
  }
}
