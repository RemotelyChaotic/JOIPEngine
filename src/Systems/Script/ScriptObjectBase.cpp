#include "ScriptObjectBase.h"
#include "ScriptRunnerSignalEmiter.h"
#include "Systems/Project.h"
#include <QEventLoop>

CScriptObjectBase::CScriptObjectBase(QPointer<CScriptRunnerSignalEmiter> pEmitter) :
  QObject(nullptr),
  m_spProject(nullptr),
  m_pSignalEmitter(pEmitter)
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

  switch (m_pSignalEmitter->ScriptExecutionStatus())
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
      if (!m_pSignalEmitter.isNull())
      {
        QEventLoop loop;
        connect(m_pSignalEmitter, &CScriptRunnerSignalEmiter::interrupt,
                &loop, &QEventLoop::quit, Qt::QueuedConnection);
        connect(m_pSignalEmitter, &CScriptRunnerSignalEmiter::resumeExecution,
                &loop, &QEventLoop::quit, Qt::QueuedConnection);
        loop.exec();
        if (m_pSignalEmitter->ScriptExecutionStatus() == CScriptRunnerSignalEmiter::eStopped)
        {
          return false;
        }
        else
        {
          return true;
        }
      }
      else
      {
        return false;
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
CJsScriptObjectBase::CJsScriptObjectBase(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                                         QPointer<QJSEngine> pEngine) :
  CScriptObjectBase(pEmitter),
  m_pEngine(pEngine)
{}
CJsScriptObjectBase::~CJsScriptObjectBase()
{}

//----------------------------------------------------------------------------------------
//
CEosScriptObjectBase::CEosScriptObjectBase(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                                           QPointer<CJsonInstructionSetParser> pParser) :
  CScriptObjectBase(pEmitter),
  m_pParser(pParser)
{}
CEosScriptObjectBase::~CEosScriptObjectBase()
{
}
