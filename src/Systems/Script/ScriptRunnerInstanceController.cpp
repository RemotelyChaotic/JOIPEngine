#include "ScriptRunnerInstanceController.h"
#include "ScriptRunnerSignalEmiter.h"

#include <QThread>

CScriptRunnerInstanceWorkerBase::CScriptRunnerInstanceWorkerBase(
    const QString& sName,
    std::weak_ptr<CScriptRunnerSignalContext> wpSignalEmitterContext) :
  QObject(),
  m_wpSignalEmiterContext(wpSignalEmitterContext),
  m_objectMap(),
  m_sName(sName)
{}
CScriptRunnerInstanceWorkerBase::~CScriptRunnerInstanceWorkerBase()
{
}

//----------------------------------------------------------------------------------------
//
void CScriptRunnerInstanceWorkerBase::FinishedScript(const QVariant& sRetVal)
{
  m_bRunning = 0;
  emit HandleScriptFinish(true, sRetVal);
}

//----------------------------------------------------------------------------------------
//
void CScriptRunnerInstanceWorkerBase::UnregisterComponents()
{
  ResetEngine();
  m_objectMap.clear();
}

//----------------------------------------------------------------------------------------
//
CScriptRunnerInstanceController::CScriptRunnerInstanceController(
    const QString& sName,
    std::shared_ptr<CScriptRunnerInstanceWorkerBase> spWorkerBase,
    std::weak_ptr<CScriptRunnerSignalContext> wpSignalEmitterContext) :
  QObject(nullptr),
  m_spWorker(spWorkerBase),
  m_pThread(new QThread(this))
{
  qRegisterMetaType<CScriptRunnerSignalEmiter*>();

  connect(m_spWorker.get(), &CScriptRunnerInstanceWorkerBase::HandleScriptFinish,
          this, &CScriptRunnerInstanceController::SlotHandleScriptFinish, Qt::QueuedConnection);
  connect(m_spWorker.get(), &CScriptRunnerInstanceWorkerBase::SignalSceneLoaded,
          this, &CScriptRunnerInstanceController::SignalSceneLoaded);

  connect(m_spWorker.get(), &CScriptRunnerInstanceWorkerBase::SignalOverlayCleared,
          this, &CScriptRunnerInstanceController::SignalOverlayCleared, Qt::QueuedConnection);
  connect(m_spWorker.get(), &CScriptRunnerInstanceWorkerBase::SignalOverlayClosed,
          this, &CScriptRunnerInstanceController::SignalOverlayClosed, Qt::QueuedConnection);
  connect(m_spWorker.get(), &CScriptRunnerInstanceWorkerBase::SignalOverlayRunAsync,
          this, &CScriptRunnerInstanceController::SignalOverlayRunAsync, Qt::QueuedConnection);

  m_pThread->setObjectName("ScriptController::"+sName);
  m_pThread->start();
  while (!m_pThread->isRunning())
  {
    thread()->wait(5);
  }
  m_spWorker->moveToThread(m_pThread.data());
  bool bOk = QMetaObject::invokeMethod(m_spWorker.get(), "Init", Qt::BlockingQueuedConnection);
  assert(bOk); Q_UNUSED(bOk)
}

//----------------------------------------------------------------------------------------
//
CScriptRunnerInstanceController::~CScriptRunnerInstanceController()
{
  bool bOk = QMetaObject::invokeMethod(m_spWorker.get(), "Deinit", Qt::BlockingQueuedConnection);
  assert(bOk); Q_UNUSED(bOk)
  m_pThread->quit();
  while (!m_pThread->isFinished())
  {
    m_pThread->wait(5);
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptRunnerInstanceController::InterruptExecution()
{
  // direct call, or else we can't stop the script
  m_spWorker->InterruptExecution();
}

//----------------------------------------------------------------------------------------
//
bool CScriptRunnerInstanceController::IsRunning() const
{
  return m_spWorker->m_bRunning == 1;
}

//----------------------------------------------------------------------------------------
//
void CScriptRunnerInstanceController::RegisterNewComponent(const QString sName,
                                                           CScriptRunnerSignalEmiter* pObject)
{
  bool bOk = QMetaObject::invokeMethod(m_spWorker.get(), "RegisterNewComponent", Qt::BlockingQueuedConnection,
                                       Q_ARG(QString, sName),
                                       Q_ARG(CScriptRunnerSignalEmiter*, pObject));
  assert(bOk); Q_UNUSED(bOk)
}

//----------------------------------------------------------------------------------------
//
void CScriptRunnerInstanceController::RunScript(const QString& sScript,
                                                tspScene spScene, tspResource spResource)
{
  bool bOk = QMetaObject::invokeMethod(m_spWorker.get(), "RunScript", Qt::QueuedConnection,
                                       Q_ARG(QString, sScript),
                                       Q_ARG(tspScene, spScene),
                                       Q_ARG(tspResource, spResource));
  assert(bOk); Q_UNUSED(bOk)
}

//----------------------------------------------------------------------------------------
//
void CScriptRunnerInstanceController::ResetEngine()
{
  bool bOk = QMetaObject::invokeMethod(m_spWorker.get(), "ResetEngine", Qt::BlockingQueuedConnection);
  assert(bOk); Q_UNUSED(bOk)
}

//----------------------------------------------------------------------------------------
//
void CScriptRunnerInstanceController::UnregisterComponents()
{
  bool bOk = QMetaObject::invokeMethod(m_spWorker.get(), "UnregisterComponents", Qt::BlockingQueuedConnection);
  assert(bOk); Q_UNUSED(bOk)
}

//----------------------------------------------------------------------------------------
//
void CScriptRunnerInstanceController::SlotHandleScriptFinish(bool bSuccess,
                                                             const QVariant& sRetVal)
{
  emit HandleScriptFinish(objectName(), bSuccess, sRetVal);
}
