#include "ScriptObjectBase.h"
#include "ScriptRunnerSignalEmiter.h"
#include "Systems/Project.h"

CScriptObjectBase::CScriptObjectBase(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                                     QPointer<QJSEngine> pEngine) :
  QObject(nullptr),
  m_spProject(nullptr),
  m_pSignalEmitter(pEmitter),
  m_pEngine(pEngine)
{

}

CScriptObjectBase::~CScriptObjectBase()
{

}

//----------------------------------------------------------------------------------------
//
void CScriptObjectBase::Cleanup()
{
  Cleanup_Impl();
}

//----------------------------------------------------------------------------------------
//
void CScriptObjectBase::SetCurrentProject(tspProject spProject)
{
  m_spProject = spProject;
}

//----------------------------------------------------------------------------------------
//
bool CScriptObjectBase::CheckIfScriptCanRun()
{
  if (nullptr == m_pSignalEmitter)
  {
    return false;
  }

  if (m_pSignalEmitter->ScriptExecutionStatus() == CScriptRunnerSignalEmiter::eStopped)
  {
    return false;
  }
  else
  {
    return true;
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptObjectBase::Cleanup_Impl()
{
  // default implementation does nothing
}
