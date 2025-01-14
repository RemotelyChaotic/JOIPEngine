#include "CommandConnectionAdded.h"
#include "FlowScene.h"
#include "Editor/EditorCommandIds.h"

CCommandConnectionAdded::CCommandConnectionAdded(QPointer<CFlowScene> pFlowScene,
                                                 const QUuid& uuid,
                                                 const QJsonObject& serialized,
                                                 QUndoCommand* pParent) :
  QUndoCommand("Added connection: " + uuid.toString(), pParent),
  m_pFlowScene(pFlowScene),
  m_bFirstInsertGuard(true),
  m_connId(uuid),
  m_connection(serialized)
{

}
CCommandConnectionAdded::~CCommandConnectionAdded()
{

}

//----------------------------------------------------------------------------------------
//
void CCommandConnectionAdded::undo()
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

//----------------------------------------------------------------------------------------
//
void CCommandConnectionAdded::redo()
{
  if (m_bFirstInsertGuard)
  {
    m_bFirstInsertGuard = false;
  }
  else
  {
    if (nullptr != m_pFlowScene)
    {
      CSceneUndoOperationLocker locker(m_pFlowScene);
      m_pFlowScene->restoreConnection(m_connection);
    }
  }
}

//----------------------------------------------------------------------------------------
//
int CCommandConnectionAdded::id() const
{
  return EEditorCommandId::eNone;
}

//----------------------------------------------------------------------------------------
//
bool CCommandConnectionAdded::mergeWith(const QUndoCommand* pOther)
{
  Q_UNUSED(pOther);
  return false;
}
