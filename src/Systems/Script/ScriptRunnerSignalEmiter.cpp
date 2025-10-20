#include "ScriptRunnerSignalEmiter.h"
#include "ScriptObjectBase.h"

#include <QDebug>

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
  if (m_bScriptExecutionStatus != status)
  {
    auto oldStatus = m_bScriptExecutionStatus;
    m_bScriptExecutionStatus = status;
    if (CScriptRunnerSignalEmiter::ScriptExecStatus::eRunning == oldStatus &&
        CScriptRunnerSignalEmiter::ScriptExecStatus::ePaused == status)
    {
      emit pauseExecution();
    }
    else if (CScriptRunnerSignalEmiter::ScriptExecStatus::ePaused == oldStatus &&
             CScriptRunnerSignalEmiter::ScriptExecStatus::eRunning == status)
    {
      emit resumeExecution();
    }
    else if (CScriptRunnerSignalEmiter::ScriptExecStatus::eStopped == oldStatus &&
             CScriptRunnerSignalEmiter::ScriptExecStatus::eRunning == status)
    {
      emit startExecution();
    }
    else if (CScriptRunnerSignalEmiter::ScriptExecStatus::eStopped != oldStatus &&
             CScriptRunnerSignalEmiter::ScriptExecStatus::eStopped == status)
    {
      emit stopExecution();
    }
  }
}

//----------------------------------------------------------------------------------------
//
CScriptRunnerSignalEmiter::ScriptExecStatus CScriptRunnerSignalContext::ScriptExecutionStatus() const
{
  return static_cast<CScriptRunnerSignalEmiter::ScriptExecStatus>(
        static_cast<qint32>(m_bScriptExecutionStatus));
}

//----------------------------------------------------------------------------------------
//
CScriptRunnerSignalEmiterAccessor::CScriptRunnerSignalEmiterAccessor(CScriptRunnerSignalEmiter* pParent) :
  m_pParent(pParent),
  m_mutex(QMutex::Recursive)
{
}
CScriptRunnerSignalEmiter* CScriptRunnerSignalEmiterAccessor::Get() const
{
  return m_pParent;
}

//----------------------------------------------------------------------------------------
//
void CScriptRunnerSignalEmiterAccessor::Lock()
{
  m_mutex.lock();
}

//----------------------------------------------------------------------------------------
//
void CScriptRunnerSignalEmiterAccessor::Unlock()
{
  m_mutex.unlock();
}

//----------------------------------------------------------------------------------------
//
CScriptRunnerSignalEmiter::CScriptRunnerSignalEmiter() :
  QObject(),
  m_spAccessor(std::make_shared<CScriptRunnerSignalEmiterAccessor>(this)),
  m_spCommunicator()
{
}

CScriptRunnerSignalEmiter::~CScriptRunnerSignalEmiter()
{
  m_spAccessor->Lock();
  m_spAccessor->m_pParent = nullptr;
  m_spAccessor->Unlock();
}

//----------------------------------------------------------------------------------------
//
std::weak_ptr<CScriptCommunicator> CScriptRunnerSignalEmiter::Communicator() const
{
  return m_spCommunicator;
}

//----------------------------------------------------------------------------------------
//
void CScriptRunnerSignalEmiter::Initialize(std::shared_ptr<CScriptRunnerSignalContext> spContext)
{
  if (m_wpContext.expired())
  {
    m_wpContext = spContext;
    if (nullptr != spContext)
    {
      connect(this, &CScriptRunnerSignalEmiter::executionError,
              spContext.get(), &CScriptRunnerSignalContext::executionError, Qt::DirectConnection);
      connect(this, &CScriptRunnerSignalEmiter::showError,
              spContext.get(), &CScriptRunnerSignalContext::showError, Qt::DirectConnection);
    }

    CreateCommunicator();
  }
}
//----------------------------------------------------------------------------------------
//
CScriptRunnerSignalEmiter::ScriptExecStatus CScriptRunnerSignalEmiter::ScriptExecutionStatus() const
{
  if (auto spContext = m_wpContext.lock())
  {
    return static_cast<CScriptRunnerSignalEmiter::ScriptExecStatus>(
          static_cast<qint32>(spContext->ScriptExecutionStatus()));
  }
  return CScriptRunnerSignalEmiter::ScriptExecStatus::eStopped;
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CScriptCommunicator> CScriptRunnerSignalEmiter::CreateCommunicator()
{
  m_spCommunicator = CreateCommunicatorImpl(m_spAccessor);
  assert(nullptr != m_spCommunicator && "Communicator was nullptr");
  assert(!m_wpContext.expired() && "Context was nullptr");

  if (auto spContext = m_wpContext.lock(); nullptr != spContext && nullptr != m_spCommunicator)
  {
    connect(spContext.get(), &CScriptRunnerSignalContext::stopExecution,
            this, [this](){
              QMutexLocker l(&m_spCommunicator->m_mutex);
              for (auto& fn : m_spCommunicator->m_vspfnStopCallback)
              {
                if (auto spFn = fn.lock())
                {
                  (*spFn)();
                }
              }
            }, Qt::DirectConnection);
    connect(spContext.get(), &CScriptRunnerSignalContext::pauseExecution,
            this, [this](){
              QMutexLocker l(&m_spCommunicator->m_mutex);
              for (auto& fn : m_spCommunicator->m_vspfnPauseCallback)
              {
                if (auto spFn = fn.lock())
                {
                  (*spFn)();
                }
              }
            }, Qt::DirectConnection);
    connect(spContext.get(), &CScriptRunnerSignalContext::resumeExecution,
            this, [this](){
              QMutexLocker l(&m_spCommunicator->m_mutex);
              for (auto& fn : m_spCommunicator->m_vspfnResumeCallback)
              {
                if (auto spFn = fn.lock())
                {
                  (*spFn)();
                }
              }
            }, Qt::DirectConnection);
  }
  else
  {
    qWarning() << tr("Context or Communicator was nullptr.");
  }
  return m_spCommunicator;
}

//----------------------------------------------------------------------------------------
//
CScriptCommunicator::CScriptCommunicator(const std::weak_ptr<CScriptRunnerSignalEmiterAccessor>& spEmitter) :
  m_mutex(QMutex::Recursive),
  m_wpSignalEmitterAccess(spEmitter)
{
}
CScriptCommunicator::~CScriptCommunicator() = default;

//----------------------------------------------------------------------------------------
//
void CScriptCommunicator::RegisterResumeCallback(const std::shared_ptr<std::function<void()>>& spFn)
{
  RegisterCallback(&m_vspfnResumeCallback, spFn);
}

void CScriptCommunicator::RegisterPauseCallback(const std::shared_ptr<std::function<void()>>& spFn)
{
  RegisterCallback(&m_vspfnPauseCallback, spFn);
}

void CScriptCommunicator::RegisterStopCallback(const std::shared_ptr<std::function<void()>>& spFn)
{
  RegisterCallback(&m_vspfnStopCallback, spFn);
}

void CScriptCommunicator::RemoveResumeCallback(const std::shared_ptr<std::function<void()>>& spFn)
{
  RemoveCallback(&m_vspfnResumeCallback, spFn);
}

void CScriptCommunicator::RemovePauseCallback(const std::shared_ptr<std::function<void()>>& spFn)
{
  RemoveCallback(&m_vspfnPauseCallback, spFn);
}

void CScriptCommunicator::RemoveStopCallback(const std::shared_ptr<std::function<void()>>& spFn)
{
  RemoveCallback(&m_vspfnStopCallback, spFn);
}

//----------------------------------------------------------------------------------------
//
void CScriptCommunicator::RegisterCallback(
  std::vector<std::weak_ptr<std::function<void()>>>* vspfnCallbacks,
  const std::shared_ptr<std::function<void()>>& spFn)
{
  QMutexLocker l(&m_mutex);
  auto it = std::find_if(vspfnCallbacks->begin(), vspfnCallbacks->end(),
                         [&spFn](const std::weak_ptr<std::function<void()>>& wpFn) {
      return wpFn.lock() == spFn;
  });
  if (vspfnCallbacks->end() == it)
  {
    vspfnCallbacks->push_back(spFn);
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptCommunicator::RemoveCallback(
  std::vector<std::weak_ptr<std::function<void()>>>* vspfnCallbacks,
  const std::shared_ptr<std::function<void()>>& spFn)
{
  QMutexLocker l(&m_mutex);
  auto it = std::find_if(vspfnCallbacks->begin(), vspfnCallbacks->end(),
                         [&spFn](const std::weak_ptr<std::function<void()>>& wpFn) {
                           return wpFn.lock() == spFn;
                         });
  if (vspfnCallbacks->end() != it)
  {
    vspfnCallbacks->erase(it);
  }
}

//----------------------------------------------------------------------------------------
//
CScriptRunnerSignalEmiter::ScriptExecStatus CScriptCommunicator::ScriptExecutionStatus() const
{
  if (auto spEmitter = LockedEmitter<CScriptRunnerSignalEmiter>())
  {
    return spEmitter->ScriptExecutionStatus();
  }
  return CScriptRunnerSignalEmiter::ScriptExecStatus::eStopped;
}
