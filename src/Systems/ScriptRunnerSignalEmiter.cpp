#include "ScriptRunnerSignalEmiter.h"

CScriptRunnerSignalEmiter::CScriptRunnerSignalEmiter() :
  QObject(),
  m_bScriptExecutionStatus(EScriptExecutionStatus(EScriptExecutionStatus::eRunning)._to_integral())
{
}

CScriptRunnerSignalEmiter::~CScriptRunnerSignalEmiter()
{
}

//----------------------------------------------------------------------------------------
//
void CScriptRunnerSignalEmiter::SetScriptExecutionStatus(EScriptExecutionStatus status)
{
  m_bScriptExecutionStatus = status._to_integral();
}

//----------------------------------------------------------------------------------------
//
EScriptExecutionStatus CScriptRunnerSignalEmiter::ScriptExecutionStatus()
{
  return EScriptExecutionStatus::_from_integral(m_bScriptExecutionStatus);
}
