#include "CommandNodeMoved.h"
#include "NodeEditorFlowView.h"
#include "NodeEditorFlowScene.h"

#include "Editor/EditorCommandIds.h"
#include "Editor/EditorWidgetTypes.h"

#include <nodes/Node>

CCommandNodeMoved::CCommandNodeMoved(QPointer<CFlowView> pFlowView,
                                     const QUuid& id,
                                     const QPointF& from,
                                     const QPointF& to,
                                     QUndoCommand* pParent) :
  QUndoCommand(QString("Node %1 moved -> %2").arg(id.toString()).arg(QString("(x:%1, y:%2)").arg(to.x()).arg(to.y())), pParent),
  m_pFlowView(pFlowView),
  m_pScene(nullptr != pFlowView ? pFlowView->Scene() : nullptr),
  m_nodeId(id),
  m_from(from),
  m_to(to)
{

}
CCommandNodeMoved::~CCommandNodeMoved()
{

}

//----------------------------------------------------------------------------------------
//
void CCommandNodeMoved::undo()
{
  DoUndoRedo(m_from);
}

//----------------------------------------------------------------------------------------
//
void CCommandNodeMoved::redo()
{
  DoUndoRedo(m_to);
}

//----------------------------------------------------------------------------------------
//
int CCommandNodeMoved::id() const
{
  return EEditorCommandId::eMoveNodeItem;
}

//----------------------------------------------------------------------------------------
//
bool CCommandNodeMoved::mergeWith(const QUndoCommand* pOther)
{
  if (nullptr == pOther || id() != pOther->id())
  {
    return false;
  }

  const CCommandNodeMoved* pOtherCasted = dynamic_cast<const CCommandNodeMoved*>(pOther);
  if (nullptr == pOtherCasted) { return false; }
  if (m_nodeId != pOtherCasted->m_nodeId) { return false; }

  m_to = pOtherCasted->m_to;
  setText(QString("Node %1 moved -> %2")
          .arg(m_nodeId.toString())
          .arg(QString("(x:%1, y:%2)").arg(m_to.x()).arg(m_to.y())));
  return true;
}

//----------------------------------------------------------------------------------------
//
void CCommandNodeMoved::DoUndoRedo(const QPointF& point)
{
  if (nullptr != m_pScene && nullptr != m_pFlowView)
  {
    const auto& nodes = m_pScene->nodes();

    auto it = nodes.find(m_nodeId);

    if (nodes.end() != it && nullptr != it->second)
    {
      if (auto pScene = dynamic_cast<CNodeEditorFlowScene*>(m_pScene.data()))
      {
        it->second->nodeGraphicsObject().setProperty(editor::c_sPropertyOldValue, point);
        CSceneUndoOperationLocker locker(pScene);
        it->second->nodeGraphicsObject().setPos(point);
      }
    }
  }
}
