#include "ScriptObjectBase.h"
#include "ScriptRunnerSignalEmiter.h"
#include "Systems/Project.h"
#include <QtLua/State>
#include <QEventLoop>
#include <QThread>

CScriptObjectBase::CScriptObjectBase(std::weak_ptr<CScriptCommunicator> wpCommunicator) :
  QObject(nullptr),
  m_spProject(nullptr),
  m_wpCommunicator(wpCommunicator)
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
  auto spComm = m_wpCommunicator.lock();
  if (nullptr == spComm)
  {
    return false;
  }

  switch (spComm->ScriptExecutionStatus())
  {
    case CScriptRunnerSignalEmiter::eStopped:
    {
      return false;
    }
    case CScriptRunnerSignalEmiter::eRunning:
    {
      return true;
    }
    case CScriptRunnerSignalEmiter::ePaused:
    {
      CScriptRunnerSignalEmiter::ScriptExecStatus status = CScriptRunnerSignalEmiter::ePaused;
      do
      {
        QThread::sleep(10);
        status = spComm->ScriptExecutionStatus();
      }
      while (CScriptRunnerSignalEmiter::ePaused == status);

      if (CScriptRunnerSignalEmiter::eStopped == status)
      {
        return false;
      }
      else
      {
        return true;
      }
    }
    default: return false;
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptObjectBase::Cleanup_Impl()
{
  // default implementation does nothing
}

//----------------------------------------------------------------------------------------
//
CJsScriptObjectBase::CJsScriptObjectBase(std::weak_ptr<CScriptCommunicator> pCommunicator,
                                         QPointer<QJSEngine> pEngine) :
  CScriptObjectBase(pCommunicator),
  m_pEngine(pEngine)
{}
CJsScriptObjectBase::CJsScriptObjectBase(std::weak_ptr<CScriptCommunicator> pCommunicator,
                                         QtLua::State* pState) :
  CScriptObjectBase(pCommunicator),
  m_pState(pState)
{}
CJsScriptObjectBase::~CJsScriptObjectBase()
{}
//----------------------------------------------------------------------------------------
//
CEosScriptObjectBase::CEosScriptObjectBase(std::weak_ptr<CScriptCommunicator> pCommunicator,
                                           QPointer<CJsonInstructionSetParser> pParser) :
  CScriptObjectBase(pCommunicator),
  m_pParser(pParser)
{}
CEosScriptObjectBase::~CEosScriptObjectBase()
{
}
