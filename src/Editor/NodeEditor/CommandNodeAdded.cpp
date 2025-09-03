#include "CommandNodeAdded.h"
#include "NodeEditorFlowScene.h"
#include "NodeEditorFlowView.h"

#include "Editor/EditorCommandIds.h"

#include <nodes/FlowScene>
#include <nodes/Node>
#include <QDebug>

CCommandNodeAdded::CCommandNodeAdded(QPointer<CFlowView> pFlowView,
                                     const QString& sModelName,
                                     const QPointF& addPoint,
                                     QPointer<QUndoStack> pUndoStack,
                                     QUndoCommand* pParent) :
  QUndoCommand("Added node: " + sModelName, pParent),
  m_pFlowView(pFlowView),
  m_pScene(nullptr != pFlowView ? pFlowView->Scene() : nullptr),
  m_pUndoStack(pUndoStack),
  m_sModelName(sModelName),
  m_addPoint(addPoint),
  m_bFirstRedo(true)
{
}
CCommandNodeAdded::CCommandNodeAdded(QPointer<CFlowView> pFlowView,
                                     QUuid nodeId,
                                     QPointer<QUndoStack> pUndoStack,
                                     QUndoCommand* pParent) :
  QUndoCommand("Added node: " + nodeId.toString(), pParent),
  m_pFlowView(pFlowView),
  m_pScene(nullptr != pFlowView ? pFlowView->Scene() : nullptr),
  m_pUndoStack(pUndoStack),
  m_nodeId(nodeId),
  m_bFirstRedo(true)
{
  if (!m_nodeId.isNull())
  {
    const auto& nodes = m_pScene->nodes();
    for (const auto& it : nodes)
    {
      if (it.first == m_nodeId)
      {
        m_addPoint = it.second->nodeGraphicsObject().pos();
        m_node = it.second->save();
        break;
      }
    }
  }
}
CCommandNodeAdded::~CCommandNodeAdded()
{
}

//----------------------------------------------------------------------------------------
//
void CCommandNodeAdded::undo()
{
  if (nullptr != m_pFlowView && nullptr != m_pScene)
  {
    const auto& map = m_pScene->nodes();

    const auto& it = map.find(m_nodeId);

    if (map.end() != it && nullptr != it->second)
    {
      m_pScene->removeNode(*it->second);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CCommandNodeAdded::redo()
{
  if (nullptr != m_pFlowView && nullptr != m_pScene)
  {
    if (m_pFlowView->IsModelHiddenInContextMenu(m_sModelName))
    {
      return;
    }

    if (m_nodeId.isNull() && m_bFirstRedo)
    {
      auto type = m_pScene->registry().create(m_sModelName);

      if (type)
      {
        auto& node = m_pScene->createNode(std::move(type));

        QPointF posView = m_pFlowView->mapToScene(QPoint(m_addPoint.x(), m_addPoint.y()));

        node.nodeGraphicsObject().setPos(posView);

        m_nodeId = node.id();
        m_node = node.save();

        m_pScene->nodePlaced(node);
      }
      else
      {
        qWarning() << "Model not found.";
      }
    }
    else if (!m_bFirstRedo)
    {
      if (auto pScene = dynamic_cast<CNodeEditorFlowScene*>(m_pScene.data()))
      {
        pScene->SetUndoOperationInProgress(true);
        pScene->restoreNode(m_node);
        pScene->SetUndoOperationInProgress(false);
      }
    }
  }

  m_bFirstRedo = false;
}

//----------------------------------------------------------------------------------------
//
int CCommandNodeAdded::id() const
{
  return EEditorCommandId::eAddNodeItem;
}

//----------------------------------------------------------------------------------------
//
bool CCommandNodeAdded::mergeWith(const QUndoCommand* pOther)
{
  if (nullptr == pOther || id() != pOther->id())
  {
    return false;
  }

  const CCommandNodeAdded* pOtherCasted = dynamic_cast<const CCommandNodeAdded*>(pOther);
  if (nullptr == pOtherCasted) { return false; }
  if (m_nodeId != pOtherCasted->m_nodeId) { return false; }

  return true;
}
