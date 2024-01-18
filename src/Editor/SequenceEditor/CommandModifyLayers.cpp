#include "CommandModifyLayers.h"
#include "TimelineWidget.h"

#include "Editor/EditorCommandIds.h"

CCommandAddOrRemoveSequenceLayer::CCommandAddOrRemoveSequenceLayer(
    CTimelineWidget* pParent,
    qint32 iIndex,
    const tspSequenceLayer& spLayer,
    std::function<void(qint32, const tspSequenceLayer&, tspSequence&)> fnDo,
    std::function<void(qint32, const tspSequenceLayer&, tspSequence&)> fnUndo,
    const QString& sOperation) :
    QUndoCommand(sOperation + " layer at index " + QString::number(iIndex), nullptr),
  m_pParent(pParent),
  m_fnDo(fnDo),
  m_fnUndo(fnUndo),
  m_spLayer(std::make_shared<SSequenceLayer>(*spLayer)),
  m_iIndex(iIndex)
{

}
CCommandAddOrRemoveSequenceLayer::~CCommandAddOrRemoveSequenceLayer()
{

}

//----------------------------------------------------------------------------------------
//
void CCommandAddOrRemoveSequenceLayer::undo()
{
  if (nullptr == m_pParent) { return; }

  auto spSeq = m_pParent->Sequence();
  if (nullptr != spSeq && nullptr != m_fnDo)
  {
    m_fnUndo(m_iIndex, std::make_shared<SSequenceLayer>(*m_spLayer), spSeq);
  }
}

//----------------------------------------------------------------------------------------
//
void CCommandAddOrRemoveSequenceLayer::redo()
{
  if (nullptr == m_pParent) { return; }

  auto spSeq = m_pParent->Sequence();
  if (nullptr != spSeq && nullptr != m_fnDo)
  {
    m_fnDo(m_iIndex, std::make_shared<SSequenceLayer>(*m_spLayer), spSeq);
  }
}

//----------------------------------------------------------------------------------------
//
int CCommandAddOrRemoveSequenceLayer::id() const
{
  return EEditorCommandId::eAddRemoveSequenceLayer;
}

//----------------------------------------------------------------------------------------
//
bool CCommandAddOrRemoveSequenceLayer::mergeWith(const QUndoCommand* pOther)
{
  if (nullptr == pOther || id() != pOther->id())
  {
    return false;
  }

  const CCommandAddOrRemoveSequenceLayer* pOtherCasted =
      dynamic_cast<const CCommandAddOrRemoveSequenceLayer*>(pOther);
  if (nullptr == pOtherCasted) { return false; }

  // currently not possible
  return false;
}
