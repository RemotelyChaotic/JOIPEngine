#include "CommandNodeEdited.h"
#include "FlowScene.h"
#include "IUndoStackAwareModel.h"
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
  return EEditorCommandId::eChangeNodeItem;
}

//----------------------------------------------------------------------------------------
//
bool CCommandNodeEdited::mergeWith(const QUndoCommand* pOther)
{
  if (nullptr == pOther || id() != pOther->id())
  {
    return false;
  }

  const CCommandNodeEdited* pOtherCasted = dynamic_cast<const CCommandNodeEdited*>(pOther);
  if (nullptr == pOtherCasted) { return false; }
  if (m_nodeId != pOtherCasted->m_nodeId) { return false; }

  m_newState = pOtherCasted->m_newState;
  return true;
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
      auto pModel = it->second->nodeDataModel();
      IUndoStackAwareModel* pUndoAware = dynamic_cast<IUndoStackAwareModel*>(pModel);
      if (nullptr != pUndoAware)
      {
        pUndoAware->UndoRestore(obj);
      }
      else
      {
        pModel->restore(obj);
      }
    }
  }
}
