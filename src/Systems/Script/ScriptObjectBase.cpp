#include "ScriptObjectBase.h"
#include "ScriptRunnerSignalEmiter.h"
#include "Systems/Project.h"

CScriptObjectBase::CScriptObjectBase(std::shared_ptr<CScriptRunnerSignalEmiter> spEmitter,
                                     QPointer<QJSEngine> pEngine) :
  QObject(nullptr),
  m_spSignalEmitter(spEmitter),
  m_spProject(nullptr),
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
  if (m_spSignalEmitter->ScriptExecutionStatus()._to_integral() == EScriptExecutionStatus::eStopped)
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
