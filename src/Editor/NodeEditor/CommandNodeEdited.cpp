#include "CommandNodeEdited.h"
#include "FlowScene.h"
#include "Editor/EditorCommandIds.h"
#include <nodes/Node>

CCommandNodeEdited::CCommandNodeEdited(QPointer<CFlowScene> pFlowScene,
                                       const QUuid& id,
                                       const QJsonObject& oldState,
                                       const QJsonObject& newState,
                                       QUndoCommand* pParent) :
  QUndoCommand(QString("Node %1 changed").arg(id.toString()), pParent),
  m_pFlowScene(pFlowScene),
  m_bFirstInsertGuard(true),
  m_nodeId(id),
  m_oldState(oldState),
  m_newState(newState)
{
}
CCommandNodeEdited::~CCommandNodeEdited()
{
}

//----------------------------------------------------------------------------------------
//
void CCommandNodeEdited::undo()
{
  DoUndoRedo(m_oldState);
}

//----------------------------------------------------------------------------------------
//
void CCommandNodeEdited::redo()
{
  if (m_bFirstInsertGuard)
  {
    m_bFirstInsertGuard = false;
  }
  else
  {
    DoUndoRedo(m_newState);
  }
}

//----------------------------------------------------------------------------------------
//
int CCommandNodeEdited::id() const
{
  return EEditorCommandId::eNone;
}

//----------------------------------------------------------------------------------------
//
bool CCommandNodeEdited::mergeWith(const QUndoCommand* pOther)
{
  Q_UNUSED(pOther)
  return false;
}

//----------------------------------------------------------------------------------------
//
void CCommandNodeEdited::DoUndoRedo(const QJsonObject& obj)
{
  if (nullptr != m_pFlowScene)
  {
    auto& nodes = m_pFlowScene->nodes();

    const auto& it = nodes.find(m_nodeId);

    if (nodes.end() != it && nullptr != it->second && nullptr != it->second->nodeDataModel())
    {
      it->second->nodeDataModel()->restore(obj);
    }
  }
}
