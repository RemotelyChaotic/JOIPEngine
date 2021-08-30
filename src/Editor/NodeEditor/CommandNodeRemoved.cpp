#include "CommandNodeRemoved.h"
#include "FlowView.h"
#include "Editor/EditorCommandIds.h"
#include <nodes/FlowScene>
#include <nodes/Node>

CCommandNodesRemoved::CCommandNodesRemoved(QPointer<CFlowView> pFlowView,
                                           const std::vector<QUuid>& vIds,
                                           QUndoCommand* pParent) :
  QUndoCommand(QString("Removed %1 nodes").arg(vIds.size()), pParent),
  m_pFlowView(pFlowView),
  m_pScene(nullptr != pFlowView ? pFlowView->scene() : nullptr),
  m_vIds(vIds)
{

}
CCommandNodesRemoved::~CCommandNodesRemoved()
{

}

//----------------------------------------------------------------------------------------
//
void CCommandNodesRemoved::undo()
{

}

//----------------------------------------------------------------------------------------
//
void CCommandNodesRemoved::redo()
{

}

//----------------------------------------------------------------------------------------
//
int CCommandNodesRemoved::id() const
{
  return EEditorCommandId::eNone;
}

//----------------------------------------------------------------------------------------
//
bool CCommandNodesRemoved::mergeWith(const QUndoCommand* pOther)
{
  Q_UNUSED(pOther);
  return false;
}
