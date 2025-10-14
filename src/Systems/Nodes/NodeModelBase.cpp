#include "NodeModelBase.h"
#include "FlowScene.h"

CNodeModelBase::CNodeModelBase() :
    m_id(),
    m_pScene(nullptr)
{
}

CNodeModelBase::~CNodeModelBase()
{}

//----------------------------------------------------------------------------------------
//
void CNodeModelBase::SetNodeContext(const QUuid& id, QPointer<CFlowScene> pScene)
{
  m_id = id;
  m_pScene = pScene;
}

//----------------------------------------------------------------------------------------
//
void CNodeModelBase::SetDebuggState(EDebugState state)
{
  if (m_debugState != state)
  {
    m_debugState = state;
  }
}
