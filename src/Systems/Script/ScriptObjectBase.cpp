#include "ScriptObjectBase.h"
#include "ScriptRunnerSignalEmiter.h"
#include "Systems/Database/Project.h"

#include <QtLua/State>

#include <QEventLoop>
#include <QTimer>
#include <QThread>
#include <QUuid>

CScriptObjectBase::CScriptObjectBase(std::weak_ptr<CScriptCommunicator> wpCommunicator) :
  QObject(nullptr),
  m_spProject(nullptr),
  m_wpCommunicator(wpCommunicator)
{
  m_spStopBase = std::make_shared<std::function<void()>>([this]() {
    emit SignalQuitLoopRequest();
  });
  if (auto spComm = m_wpCommunicator.lock())
  {
    spComm->RegisterStopCallback(m_spStopBase);
  }
}

CScriptObjectBase::~CScriptObjectBase()
{
  if (auto spComm = m_wpCommunicator.lock())
  {
    spComm->RemoveStopCallback(m_spStopBase);
  }
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
QVariant CScriptObjectBase::RequestValue(const QString& sKey)
{
  if (!CheckIfScriptCanRun()) { return QVariant(); }

  QString sRequestId = QUuid::createUuid().toString();

  QTimer::singleShot(0, this, [this,sKey,sRequestId]() {
    if (auto spComm = m_wpCommunicator.lock())
    {
      if (auto spSignalEmitter = spComm->LockedEmitter<CScriptRunnerSignalEmiter>())
      {
        emit spSignalEmitter->requestValue(sKey, sRequestId);
      }
    }
  });

  // local loop to wait for answer
  QPointer<CScriptObjectBase> pThis(this);
  std::shared_ptr<QVariant> spReturnValue = std::make_shared<QVariant>();
  QEventLoop loop;
  QMetaObject::Connection quitLoop =
    connect(this, &CScriptObjectBase::SignalQuitLoopRequest, &loop, &QEventLoop::quit);
  QMetaObject::Connection interruptThisLoop =
    connect(this, &CScriptObjectBase::SignalInterruptExecution,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
  QMetaObject::Connection showRetValLoop;
  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CScriptRunnerSignalEmiter>())
    {
      showRetValLoop =
        connect(spSignalEmitter.Get(), &CScriptRunnerSignalEmiter::requestValueRet,
                &loop, [&loop, spReturnValue, sRequestId](QJSValue var, QString sRequestIdRet)
                {
                  if (sRequestId == sRequestIdRet)
                  {
                    *spReturnValue = var.toVariant();
                    spReturnValue->detach(); // fixes some crashes with QJSEngine
                    bool bOk = QMetaObject::invokeMethod(&loop, "quit", Qt::QueuedConnection);
                    assert(bOk); Q_UNUSED(bOk)
                  }
                  // direct connection to fix cross thread issues with QString content being deleted
                }, Qt::DirectConnection);
    }
  }
  loop.exec();
  loop.disconnect();

  if (nullptr != pThis)
  {
    disconnect(quitLoop);
    disconnect(interruptThisLoop);
    if (showRetValLoop) disconnect(showRetValLoop);
  }

  return *spReturnValue;
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
