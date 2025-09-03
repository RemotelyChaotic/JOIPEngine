#include "CommandNodeRemoved.h"
#include "NodeEditorFlowScene.h"
#include "NodeEditorFlowView.h"

#include "Editor/EditorCommandIds.h"

#include <nodes/Node>

CCommandNodesRemoved::CCommandNodesRemoved(QPointer<CFlowView> pFlowView,
                                           const std::vector<QUuid>& vIds,
                                           QPointer<QUndoStack> pUndoStack,
                                           QUndoCommand* pParent) :
  QUndoCommand(QString("Removed %1 nodes").arg(vIds.size()), pParent),
  m_pFlowView(pFlowView),
  m_pScene(nullptr != pFlowView ? pFlowView->Scene() : nullptr),
  m_pUndoStack(pUndoStack),
  m_vIds(vIds),
  m_nodesSaved()
{
}
CCommandNodesRemoved::~CCommandNodesRemoved()
{

}

//----------------------------------------------------------------------------------------
//
void CCommandNodesRemoved::undo()
{
  if (nullptr != m_pScene && nullptr != m_pFlowView)
  {
    for (qint32 i = 0; m_vIds.size() > static_cast<size_t>(i); ++i)
    {
      auto it = m_nodesSaved.find(m_vIds[static_cast<size_t>(i)]);

      if (m_nodesSaved.end() != it)
      {
        if (auto pScene = dynamic_cast<CNodeEditorFlowScene*>(m_pScene.data()))
        {
          pScene->SetUndoOperationInProgress(true);
          auto& node = m_pScene->restoreNode(it->second);
          pScene->SetUndoOperationInProgress(false);
          node.nodeGraphicsObject().setSelected(true);
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CCommandNodesRemoved::redo()
{
  if (nullptr != m_pScene && nullptr != m_pFlowView)
  {
    for (qint32 i = 0; m_vIds.size() > static_cast<size_t>(i); ++i)
    {
      const auto& nodes = m_pScene->nodes();

      auto it = nodes.find(m_vIds[static_cast<size_t>(i)]);

      if (nodes.end() != it && nullptr != it->second)
      {
        m_nodesSaved.insert({it->first, it->second->save()});

        m_pScene->removeNode(*it->second.get());
      }
    }
  }
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
