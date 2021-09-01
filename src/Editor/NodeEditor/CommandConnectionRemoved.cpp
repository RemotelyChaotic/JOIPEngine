#include "CommandConnectionRemoved.h"
#include "FlowScene.h"
#include "Editor/EditorCommandIds.h"

CCommandConnectionRemoved::CCommandConnectionRemoved(QPointer<CFlowScene> pFlowScene,
                                                     const QUuid& uuid,
                                                     const QJsonObject& serialized,
                                                     QUndoCommand* pParent) :
  QUndoCommand("Removed connection: " + uuid.toString(), pParent),
  m_pFlowScene(pFlowScene),
  m_bFirstInsertGuard(true),
  m_connId(uuid),
  m_connection(serialized)
{

}
CCommandConnectionRemoved::~CCommandConnectionRemoved()
{

}

//----------------------------------------------------------------------------------------
//
void CCommandConnectionRemoved::undo()
{
  if (nullptr != m_pFlowScene)
  {
    CSceneUndoOperationLocker locker(m_pFlowScene);
    m_pFlowScene->restoreConnection(m_connection);
  }
}

//----------------------------------------------------------------------------------------
//
void CCommandConnectionRemoved::redo()
{
  if (m_bFirstInsertGuard)
  {
    m_bFirstInsertGuard = false;
  }
  else
  {
    if (nullptr != m_pFlowScene)
    {
      auto& map = m_pFlowScene->connections();
      auto it = map.find(m_connId);
      if (map.end() != it && nullptr != it->second)
      {
        CSceneUndoOperationLocker locker(m_pFlowScene);
        m_pFlowScene->deleteConnection(*it->second);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
int CCommandConnectionRemoved::id() const
{
  return EEditorCommandId::eNone;
}

//----------------------------------------------------------------------------------------
//
bool CCommandConnectionRemoved::mergeWith(const QUndoCommand* pOther)
{
  Q_UNUSED(pOther);
  return false;
}
