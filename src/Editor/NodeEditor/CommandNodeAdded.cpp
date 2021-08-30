#include "CommandNodeAdded.h"
#include "FlowView.h"
#include "Editor/EditorCommandIds.h"
#include <nodes/FlowScene>
#include <nodes/Node>
#include <QDebug>

CCommandNodeAdded::CCommandNodeAdded(QPointer<CFlowView> pFlowView,
                                     const QString& sModelName,
                                     const QPoint& addPoint,
                                     QUndoCommand* pParent) :
  QUndoCommand("Added node: " + sModelName, pParent),
  m_pFlowView(pFlowView),
  m_pScene(nullptr != pFlowView ? pFlowView->scene() : nullptr),
  m_sModelName(sModelName),
  m_addPoint(addPoint)
{
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

    auto type = m_pScene->registry().create(m_sModelName);

    if (type)
    {
      if (m_nodeId.isNull())
      {
        auto& node = m_pScene->createNode(std::move(type));

        QPointF posView = m_pFlowView->mapToScene(m_addPoint);

        node.nodeGraphicsObject().setPos(posView);

        m_nodeId = node.id();
        m_node = node.save();

        m_pScene->nodePlaced(node);
      }
      else
      {
        m_pScene->restoreNode(m_node);
      }
    }
    else
    {
      qDebug() << "Model not found";
    }
  }
}

//----------------------------------------------------------------------------------------
//
int CCommandNodeAdded::id() const
{
  return EEditorCommandId::eNone;
}

//----------------------------------------------------------------------------------------
//
bool CCommandNodeAdded::mergeWith(const QUndoCommand* pOther)
{
  Q_UNUSED(pOther);
  return false;
}
