#include "SequenceRunner.h"
#include "ScriptObjectBase.h"
#include "ScriptRunnerInstanceController.h"
#include "ScriptRunnerSignalEmiter.h"

#include "Systems/Resource.h"
#include "Systems/Sequence/ISequenceObjectRunner.h"
#include "Systems/Sequence/Sequence.h"

#include <QDebug>
#include <QJsonDocument>
#include <QTimer>
#include <QThread>

#include <chrono>
#include <queue>

namespace
{
  constexpr qint64 c_iTimerInterval = 8; //~120/s
}

//----------------------------------------------------------------------------------------
//
namespace
{
  struct STimedInstruction
  {
    sequence::tTimePos m_iTime;
    QString m_sLayerType;
    QString m_sLayerName;
    std::shared_ptr<SSequenceInstruction> m_spInstr;
  };
}

//----------------------------------------------------------------------------------------
//
class CSequenceRunnerInstanceWorker : public CScriptRunnerInstanceWorkerBase
{
  Q_OBJECT
public:
  CSequenceRunnerInstanceWorker(const QString& sName,
                                std::weak_ptr<CScriptRunnerSignalContext> wpSignalEmitterContext) :
      CScriptRunnerInstanceWorkerBase(sName, wpSignalEmitterContext),
      m_sName(sName)
  {
    if (auto spSignalEmitter = wpSignalEmitterContext.lock())
    {
      connect(spSignalEmitter.get(), &CScriptRunnerSignalContext::pauseExecution,
              this, &CSequenceRunnerInstanceWorker::PauseExecution);
      connect(spSignalEmitter.get(), &CScriptRunnerSignalContext::resumeExecution,
              this, &CSequenceRunnerInstanceWorker::ResumeExecution);
    }
  }
  ~CSequenceRunnerInstanceWorker(){}

public slots:
  void HandleError(const QString& sValue);
  void Init() override;
  void Deinit() override;
  void InterruptExecution() override;
  void PauseExecution();
  void ResumeExecution();
  void RunScript(const QString& sScript,
                 tspScene spScene, tspResource spResource) override;
  void RegisterNewComponent(const QString sName, CScriptRunnerSignalEmiter* pObject) override;
  void ResetEngine() override;
  void TimerTimeout();

private:
  void BuildInstructions(const tspSequence& spSequence);
  void RunInstruction(const STimedInstruction& instr);

  tspProject                                     m_spProject;
  QPointer<QTimer>                               m_pTimer;
  std::vector<STimedInstruction>                 m_vBuiltInstructions;
  std::chrono::time_point<std::chrono::high_resolution_clock> m_lastPoint;
  qint64                                         m_iEllapsedTime;
  QString                                        m_sName;
};

//----------------------------------------------------------------------------------------
//
void CSequenceRunnerInstanceWorker::HandleError(const QString& sValue)
{
  m_bRunning = 0;

  auto spSignalEmitterContext = m_wpSignalEmiterContext.lock();
  if (nullptr == spSignalEmitterContext)
  {
    qCritical() << "SignalEmitter is null";
    return;
  }

  qCritical() << sValue;
  emit spSignalEmitterContext->showError(sValue, QtMsgType::QtCriticalMsg);
  emit spSignalEmitterContext->executionError(sValue, -1, QString());
}

//----------------------------------------------------------------------------------------
//
void CSequenceRunnerInstanceWorker::Init()
{
  thread()->setPriority(QThread::HighPriority);
  m_pTimer = new QTimer();
  m_pTimer->setTimerType(Qt::PreciseTimer);
  m_pTimer->setSingleShot(true);
  m_pTimer->setInterval(c_iTimerInterval);
  connect(m_pTimer, &QTimer::timeout, this, &CSequenceRunnerInstanceWorker::TimerTimeout);
}

//----------------------------------------------------------------------------------------
//
void CSequenceRunnerInstanceWorker::Deinit()
{
  InterruptExecution();

  ResetEngine();

  m_pTimer->stop();
  if (nullptr != m_pTimer)
  {
    delete m_pTimer;
  }
}

//----------------------------------------------------------------------------------------
//
void CSequenceRunnerInstanceWorker::InterruptExecution()
{
  m_bRunning = 0;
  if (nullptr != m_pTimer)
  {
    m_pTimer->stop();
  }
  emit SignalInterruptExecution();
}

//----------------------------------------------------------------------------------------
//
void CSequenceRunnerInstanceWorker::PauseExecution()
{
  m_pTimer->stop();
  std::chrono::time_point<std::chrono::high_resolution_clock> newPoint =
      std::chrono::high_resolution_clock::now();
  m_iEllapsedTime +=
      std::chrono::duration_cast<std::chrono::milliseconds>(newPoint - m_lastPoint).count();
  m_lastPoint = newPoint;
}

//----------------------------------------------------------------------------------------
//
void CSequenceRunnerInstanceWorker::ResumeExecution()
{
  std::chrono::time_point<std::chrono::high_resolution_clock> newPoint =
      std::chrono::high_resolution_clock::now();
  m_lastPoint = newPoint;
  m_pTimer->start(m_pTimer->remainingTime());
}

//----------------------------------------------------------------------------------------
//
void CSequenceRunnerInstanceWorker::RunScript(const QString& sScript,
               tspScene, tspResource spResource)
{
  // set current Project
  {
    QReadLocker scriptLocker(&spResource->m_rwLock);
    m_spProject = spResource->m_spParent;
    for (auto it = m_objectMap.begin(); m_objectMap.end() != it; ++it)
    {
      it->second->SetCurrentProject(spResource->m_spParent);
    }
  }

  auto spSignalEmitterContext = m_wpSignalEmiterContext.lock();
  if (nullptr == spSignalEmitterContext)
  {
    HandleError(QString());
    return;
  }

  tspSequence spCurrentSequence = std::make_shared<SSequenceFile>();
  spCurrentSequence->FromJsonObject(QJsonDocument::fromJson(sScript.toUtf8()).object());
  BuildInstructions(spCurrentSequence);

  if (m_vBuiltInstructions.empty())
  {
    HandleError(tr("No instructions in sequence."));
    return;
  }

  // resume if interrupetd
  m_bRunning = 1;
  m_iEllapsedTime = 0;
  m_lastPoint = std::chrono::high_resolution_clock::now();

  spSignalEmitterContext->SetScriptExecutionStatus(CScriptRunnerSignalEmiter::eRunning);

  qint64 iFirstInterval = m_vBuiltInstructions.front().m_iTime;
  if (iFirstInterval > 0)
  {
    m_pTimer->start(iFirstInterval);
  }
  else
  {
    m_pTimer->start();
  }
}

//----------------------------------------------------------------------------------------
//
void CSequenceRunnerInstanceWorker::RegisterNewComponent(const QString sName, CScriptRunnerSignalEmiter* pObject)
{
  auto it = m_objectMap.find(sName);
  if (m_objectMap.end() == it)
  {
    if (nullptr != pObject)
    {
      pObject->Initialize(m_wpSignalEmiterContext.lock());
      std::shared_ptr<CScriptObjectBase> spObject =
          pObject->CreateNewSequenceObject();
      if (nullptr != spObject)
      {
        connect(this, &CSequenceRunnerInstanceWorker::SignalInterruptExecution,
                spObject.get(), &CScriptObjectBase::SignalInterruptExecution, Qt::QueuedConnection);

        if (spObject->thread() != thread())
        {
          spObject->moveToThread(thread());
        }
        m_objectMap.insert({ sName, spObject });
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CSequenceRunnerInstanceWorker::ResetEngine()
{
  if (nullptr != m_pTimer)
  {
    m_pTimer->stop();
  }

  for (auto it = m_objectMap.begin(); m_objectMap.end() != it; ++it)
  {
    it->second->Cleanup();
    it->second->SetCurrentProject(nullptr);
  }

  m_vBuiltInstructions.clear();
}

//----------------------------------------------------------------------------------------
//
void CSequenceRunnerInstanceWorker::TimerTimeout()
{
  std::chrono::time_point<std::chrono::high_resolution_clock> newPoint =
      std::chrono::high_resolution_clock::now();
  m_iEllapsedTime +=
      std::chrono::duration_cast<std::chrono::milliseconds>(newPoint - m_lastPoint).count();
  m_lastPoint = newPoint;

  if (m_vBuiltInstructions.empty())
  {
    m_bRunning = 0;
    emit HandleScriptFinish(true, QString());
    return;
  }

  auto instr = m_vBuiltInstructions.front();
  while (!m_vBuiltInstructions.empty() && instr.m_iTime <= m_iEllapsedTime)
  {
    RunInstruction(instr);
    m_vBuiltInstructions.erase(m_vBuiltInstructions.begin());
    if (!m_vBuiltInstructions.empty())
    {
      instr = m_vBuiltInstructions.front();
    }
  }

  if (m_vBuiltInstructions.empty())
  {
    m_bRunning = 0;
    emit HandleScriptFinish(true, QString());
    return;
  }

  m_pTimer->start(m_vBuiltInstructions.front().m_iTime - m_iEllapsedTime);
}

//----------------------------------------------------------------------------------------
//
void CSequenceRunnerInstanceWorker::BuildInstructions(const tspSequence& spSequence)
{
  for (const auto& spLayer : spSequence->m_vspLayers)
  {
    for (const auto& [time, spInstr] : spLayer->m_vspInstructions)
    {
      m_vBuiltInstructions.push_back({time + sequence::TimeOffsetFromInstructionType(spInstr->m_sInstructionType),
                                      spLayer->m_sLayerType, spLayer->m_sName,
                                      spInstr});
    }
  }
  std::sort(m_vBuiltInstructions.begin(), m_vBuiltInstructions.end(),
            [](const STimedInstruction& instrA, const STimedInstruction& instrB)
  {
    return instrA.m_iTime < instrB.m_iTime;
  });
}

//----------------------------------------------------------------------------------------
//
void CSequenceRunnerInstanceWorker::RunInstruction(const STimedInstruction& instr)
{
  auto it = m_objectMap.find(instr.m_sLayerName);
  if (m_objectMap.end() == it)
  {
    if (auto pRunInstr = dynamic_cast<SRunScriptInstruction*>(instr.m_spInstr.get()))
    {
      emit SignalRunAsync(m_spProject, pRunInstr->m_sResource, pRunInstr->m_sResource,
                          EScriptRunnerType::eAsync);
    }
    else if (auto pSequenceRunner = dynamic_cast<ISequenceObjectRunner*>(it->second.get()))
    {
      pSequenceRunner->RunSequenceInstruction(instr.m_sLayerName, instr.m_spInstr);
    }
  }
}

//----------------------------------------------------------------------------------------
//
CSequenceRunner::CSequenceRunner(std::weak_ptr<CScriptRunnerSignalContext> spSignalEmitterContext,
                                 tRunningScriptsCheck fnRunningScriptsCheck,
                                 QObject* pParent) :
  QObject(pParent),
  IScriptRunnerFactory(),
  m_wpSignalEmitterContext(spSignalEmitterContext),
  m_signalEmiterMutex(QMutex::Recursive),
  m_pSignalEmiters(),
  m_fnRunningScriptsCheck(fnRunningScriptsCheck),
  m_bInitialized(0)
{
}
CSequenceRunner::~CSequenceRunner()
{

}

//----------------------------------------------------------------------------------------
//
void CSequenceRunner::Initialize()
{
  m_bInitialized = 1;
}

//----------------------------------------------------------------------------------------
//
void CSequenceRunner::Deinitialize()
{
  m_bInitialized = 0;
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<IScriptRunnerInstanceController>
CSequenceRunner::LoadScript(const QString& sScript, tspScene spScene, tspResource spResource)
{
  if (nullptr == spResource)
  {
    QString sError = tr("Resource file is null");
    qCritical() << sError;
    emit SignalEmmitterContext()->showError(sError, QtMsgType::QtCriticalMsg);
    return nullptr;
  }

  std::shared_ptr<CScriptRunnerInstanceController> spRunner = CreateRunner(c_sMainRunner);
  RunScript(spRunner, sScript, spScene, spResource);
  return spRunner;
}

//----------------------------------------------------------------------------------------
//
void CSequenceRunner::RegisterNewComponent(const QString sName, QJSValue signalEmitter)
{
  CScriptRunnerSignalEmiter* pObject = nullptr;
  if (signalEmitter.isObject())
  {
    pObject = qobject_cast<CScriptRunnerSignalEmiter*>(signalEmitter.toQObject());
  }

  QMutexLocker lockerEmiter(&m_signalEmiterMutex);
  m_pSignalEmiters.insert({sName, pObject});
}

//----------------------------------------------------------------------------------------
//
void CSequenceRunner::UnregisterComponents()
{
  QMutexLocker lockerEmiter(&m_signalEmiterMutex);
  m_pSignalEmiters.clear();
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<IScriptRunnerInstanceController>
CSequenceRunner::RunAsync(const QString& sId, const QString& sScript,
                          tspResource spResource)
{
  if (nullptr == spResource)
  {
    QString sError = tr("Resource file is null");
    qCritical() << sError;
    emit SignalEmmitterContext()->showError(sError, QtMsgType::QtCriticalMsg);
    return nullptr;
  }

  std::shared_ptr<CScriptRunnerInstanceController> spRunner = CreateRunner(sId);
  RunScript(spRunner, sScript, nullptr, spResource);
  return spRunner;
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CScriptRunnerSignalContext> CSequenceRunner::SignalEmmitterContext()
{
  return m_wpSignalEmitterContext.lock();
}

//----------------------------------------------------------------------------------------
//
void CSequenceRunner::SlotHandleScriptFinish(const QString& sName, bool bSuccess, const QVariant& sRetVal)
{
  emit SignalRemoveScriptRunner(sName);

  if (!bSuccess)
  {
    qWarning() << tr("Error in sequence, unloading project.");
  }

  if (c_sMainRunner == sName || !m_fnRunningScriptsCheck())
  {
    emit SignalScriptRunFinished(bSuccess, QString());
  }
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CScriptRunnerInstanceController> CSequenceRunner::CreateRunner(const QString& sId)
{
  std::shared_ptr<CScriptRunnerInstanceController> spController =
      std::make_shared<CScriptRunnerInstanceController>(
          sId,
          std::make_shared<CSequenceRunnerInstanceWorker>(
              sId, m_wpSignalEmitterContext),
          m_wpSignalEmitterContext);
  spController->setObjectName(sId);
  connect(spController.get(), &CScriptRunnerInstanceController::HandleScriptFinish,
          this, &CSequenceRunner::SlotHandleScriptFinish);
  connect(spController.get(), &CScriptRunnerInstanceController::SignalSceneLoaded,
          this, &CSequenceRunner::SignalSceneLoaded);

  connect(spController.get(), &CScriptRunnerInstanceController::SignalClearThreads,
          this, &CSequenceRunner::SignalClearThreads);
  connect(spController.get(), &CScriptRunnerInstanceController::SignalKill,
          this, &CSequenceRunner::SignalKill);
  connect(spController.get(), &CScriptRunnerInstanceController::SignalRunAsync,
          this, &CSequenceRunner::SignalRunAsync);

  QMutexLocker lockerEmiter(&m_signalEmiterMutex);
  for (auto& itEmiter : m_pSignalEmiters)
  {
    spController->RegisterNewComponent(itEmiter.first, itEmiter.second);
  }
  return spController;
}

//----------------------------------------------------------------------------------------
//
void CSequenceRunner::RunScript(std::shared_ptr<CScriptRunnerInstanceController> spController,
                                const QString& sScript,
                                tspScene spScene, tspResource spResource)
{
  spController->RunScript(sScript, spScene, spResource);
}

#include "SequenceRunner.moc"
