#include "EditorNodeModelBase.h"
#include "FlowScene.h"

CEditorNodeModelBase::CEditorNodeModelBase() :
  m_id(),
  m_pScene(nullptr)
{
}

CEditorNodeModelBase::~CEditorNodeModelBase()
{}

//----------------------------------------------------------------------------------------
//
void CEditorNodeModelBase::SetNodeContext(const QUuid& id, QPointer<CFlowScene> pScene)
{
  m_id = id;
  m_pScene = pScene;
}
