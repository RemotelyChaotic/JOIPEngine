#include "ScriptRunnerSignalEmiter.h"
#include "ScriptObjectBase.h"

CScriptRunnerSignalContext::CScriptRunnerSignalContext() :
  m_bScriptExecutionStatus(CScriptRunnerSignalEmiter::ScriptExecStatus::eRunning)
{

}
CScriptRunnerSignalContext::~CScriptRunnerSignalContext()
{

}

//----------------------------------------------------------------------------------------
//
void CScriptRunnerSignalContext::SetScriptExecutionStatus(CScriptRunnerSignalEmiter::ScriptExecStatus status)
{
  m_bScriptExecutionStatus = status;
}

//----------------------------------------------------------------------------------------
//
CScriptRunnerSignalEmiter::ScriptExecStatus CScriptRunnerSignalContext::ScriptExecutionStatus()
{
  return static_cast<CScriptRunnerSignalEmiter::ScriptExecStatus>(
        static_cast<qint32>(m_bScriptExecutionStatus));
}

//----------------------------------------------------------------------------------------
//
CScriptRunnerSignalEmiter::CScriptRunnerSignalEmiter() :
  QObject()
{

}

CScriptRunnerSignalEmiter::CScriptRunnerSignalEmiter(const CScriptRunnerSignalEmiter& other) :
  QObject(),
  m_spContext(other.m_spContext)
{
}

CScriptRunnerSignalEmiter::~CScriptRunnerSignalEmiter()
{
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CScriptObjectBase> CScriptRunnerSignalEmiter::CreateNewScriptObject(QPointer<QJSEngine>)
{
  return nullptr;
}

std::shared_ptr<CScriptObjectBase> CScriptRunnerSignalEmiter::CreateNewScriptObject(QPointer<CJsonInstructionSetParser>)
{
  return nullptr;
}
std::shared_ptr<CScriptObjectBase> CScriptRunnerSignalEmiter::CreateNewScriptObject(QtLua::State*)
{
  return nullptr;
}
std::shared_ptr<CScriptObjectBase> CScriptRunnerSignalEmiter::CreateNewSequenceObject()
{
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
void CScriptRunnerSignalEmiter::Initialize(std::shared_ptr<CScriptRunnerSignalContext> spContext)
{
  if (nullptr == m_spContext)
  {
    m_spContext = spContext;
    if (nullptr != spContext)
    {
      connect(spContext.get(), &CScriptRunnerSignalContext::clearStorage,
              this, &CScriptRunnerSignalEmiter::clearStorage, Qt::DirectConnection);
      connect(this, &CScriptRunnerSignalEmiter::executionError,
              spContext.get(), &CScriptRunnerSignalContext::executionError, Qt::DirectConnection);
      connect(spContext.get(), &CScriptRunnerSignalContext::interrupt,
              this, &CScriptRunnerSignalEmiter::interrupt, Qt::DirectConnection);
      connect(spContext.get(), &CScriptRunnerSignalContext::pauseExecution,
              this, &CScriptRunnerSignalEmiter::pauseExecution, Qt::DirectConnection);
      connect(spContext.get(), &CScriptRunnerSignalContext::resumeExecution,
              this, &CScriptRunnerSignalEmiter::resumeExecution, Qt::DirectConnection);
      connect(this, &CScriptRunnerSignalEmiter::showError,
              spContext.get(), &CScriptRunnerSignalContext::showError, Qt::DirectConnection);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptRunnerSignalEmiter::SetScriptExecutionStatus(
    CScriptRunnerSignalEmiter::ScriptExecStatus status)
{
  if (nullptr != m_spContext)
  {
    m_spContext->m_bScriptExecutionStatus = status;
  }
}

//----------------------------------------------------------------------------------------
//
CScriptRunnerSignalEmiter::ScriptExecStatus CScriptRunnerSignalEmiter::ScriptExecutionStatus()
{
  return static_cast<CScriptRunnerSignalEmiter::ScriptExecStatus>(
        static_cast<qint32>(m_spContext->m_bScriptExecutionStatus));
}
