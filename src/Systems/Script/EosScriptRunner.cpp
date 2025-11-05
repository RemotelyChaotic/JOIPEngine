#include "EosScriptRunner.h"
#include "ScriptNotification.h"
#include "ScriptObjectBase.h"
#include "ScriptRunnerSignalEmiter.h"

#include "Systems/EOS/CommandEosEndBase.h"
#include "Systems/EOS/CommandEosGotoBase.h"
#include "Systems/EOS/CommandEosNoopBase.h"
#include "Systems/EOS/EosCommands.h"

#include "Systems/JSON/JsonInstructionSetParser.h"
#include "Systems/JSON/JsonInstructionSetRunner.h"

#include "Systems/Database/Resource.h"
#include "Systems/Database/Scene.h"
#include "Systems/ScriptRunner.h"

#include <QDebug>
#include <QFile>
#include <QTimer>

struct SFinishTag {};

//----------------------------------------------------------------------------------------
//
class CCommandEosEnd : public CCommandEosEndBase
{
public:
  CCommandEosEnd() : CCommandEosEndBase() {}
  ~CCommandEosEnd() override {}

  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) override
  {
    Q_UNUSED(args)
    return SRunRetVal<ENextCommandToCall::eFinish>(SRunRetValAnyWrapper{SFinishTag()});
  }
};

//----------------------------------------------------------------------------------------
//
class CCommandEosGoto : public CCommandEosGotoBase
{
public:
  CCommandEosGoto() : CCommandEosGotoBase() {}
  ~CCommandEosGoto() override {}

  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) override
  {
    const auto& itTarget = GetValue<EArgumentType::eString>(args, "target");
    if (HasValue(args, "target") && IsOk<EArgumentType::eString>(itTarget))
    {
      QString sTarget = std::get<QString>(itTarget);
      sTarget.replace("*", "(.*)");
      return SRunRetVal<ENextCommandToCall::eFinish>(SRunRetValAnyWrapper{sTarget});
    }
    return SJsonException{"internal Error: No target for goto.", "target", eos::c_sCommandGoto, 0, 0};
  }
};

//----------------------------------------------------------------------------------------
//
class CCommandEosNoop : public CCommandEosNoopBase
{
public:
  CCommandEosNoop() : CCommandEosNoopBase() {}
  ~CCommandEosNoop() override {}

  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) override
  {
    Q_UNUSED(args)
    return SRunRetVal<ENextCommandToCall::eSibling>();
  }
};

//----------------------------------------------------------------------------------------
//
CEosScriptRunnerInstanceController::CEosScriptRunnerInstanceController(
    const QString& sName, std::shared_ptr<CJsonInstructionSetRunner> spEosRunner) :
  QObject(),
  IScriptRunnerInstanceController(),
  m_spEosRunner(spEosRunner)
{
  Q_UNUSED(sName)
}
CEosScriptRunnerInstanceController::~CEosScriptRunnerInstanceController()
{}

//----------------------------------------------------------------------------------------
//
void CEosScriptRunnerInstanceController::InterruptExecution()
{
  m_spEosRunner->Interrupt();
}

//----------------------------------------------------------------------------------------
//
bool CEosScriptRunnerInstanceController::IsRunning() const
{
  return m_spEosRunner->IsRunning();
}

//----------------------------------------------------------------------------------------
//
void CEosScriptRunnerInstanceController::RegisterNewComponent(const QString&,
                                                              std::weak_ptr<CScriptCommunicator> wpCommunicator)
{
}

//----------------------------------------------------------------------------------------
//
void CEosScriptRunnerInstanceController::ResetEngine()
{
  m_spEosRunner->Interrupt();
}

//----------------------------------------------------------------------------------------
//
void CEosScriptRunnerInstanceController::UnregisterComponent(const QString&)
{
}

//----------------------------------------------------------------------------------------
//
void CEosScriptRunnerInstanceController::UnregisterComponents()
{
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CJsonInstructionSetRunner> CEosScriptRunnerInstanceController::Runner() const
{
  return m_spEosRunner;
}

//----------------------------------------------------------------------------------------
//
CEosScriptRunner::CEosScriptRunner(std::weak_ptr<CScriptRunnerSignalContext> spSignalEmitterContext,
                                   CScriptRunner* pRunnerParent,
                                   QObject* pParent) :
  QObject(pParent),
  IScriptRunnerFactory(),
  m_spEosParser(std::make_unique<CJsonInstructionSetParser>()),
  m_wpSignalEmitterContext(spSignalEmitterContext),
  m_sceneMutex(QMutex::Recursive),
  m_sSceneName(),
  m_objectMapMutex(QMutex::Recursive),
  m_objectMap(),
  m_pRunnerParent(pRunnerParent),
  m_bInitialized(0)
{
}

CEosScriptRunner::~CEosScriptRunner()
{}

//----------------------------------------------------------------------------------------
//
void CEosScriptRunner::Initialize()
{
  m_spEosParser->RegisterInstructionSetPath("Commands", "/");
  m_spEosParser->RegisterInstruction(eos::c_sCommandEnd, std::make_shared<CCommandEosEnd>());
  m_spEosParser->RegisterInstruction(eos::c_sCommandGoto, std::make_shared<CCommandEosGoto>());
  m_spEosParser->RegisterInstruction(eos::c_sCommandNoop, std::make_shared<CCommandEosNoop>());
  m_bInitialized = 1;
}

//----------------------------------------------------------------------------------------
//
void CEosScriptRunner::Deinitialize()
{
  m_bInitialized = 0;
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<IScriptRunnerInstanceController>
CEosScriptRunner::LoadScript(const QString& sScript, tspScene spScene, tspResource spResource)
{
  // get scene name
  if (nullptr != spScene)
  {
    QReadLocker locker(&spScene->m_rwLock);
    QMutexLocker lockerScene(&m_sceneMutex);
    m_sSceneName = spScene->m_sName;
  }

  // set current Project
  {
    QReadLocker scriptLocker(&spResource->m_rwLock);
    m_objectMapMutex.lock();
    for (auto it = m_objectMap.begin(); m_objectMap.end() != it; ++it)
    {
      it->second->SetCurrentProject(spResource->m_spParent);
    }
    m_objectMapMutex.unlock();
  }

  auto spSignalEmitterContext = m_wpSignalEmitterContext.lock();
  if (nullptr == spSignalEmitterContext)
  {
    HandleError(SJsonException{"Internal error.","","",0,0});
    return nullptr;
  }

  // create runner
  std::shared_ptr<CJsonInstructionSetRunner> spEosRunnerMain =
      m_spEosParser->ParseJson(sScript);

  if (nullptr == spEosRunnerMain)
  {
    HandleError(m_spEosParser->Error());
    return nullptr;
  }

  spEosRunnerMain->setObjectName(c_sMainRunner);
  connect(spEosRunnerMain.get(), &CJsonInstructionSetRunner::CommandRetVal,
          this, &CEosScriptRunner::SlotCommandRetVal);
  connect(spEosRunnerMain.get(), &CJsonInstructionSetRunner::Fork,
          this, &CEosScriptRunner::SlotFork);

  spSignalEmitterContext->SetScriptExecutionStatus(CScriptRunnerSignalEmiter::eRunning);

  QTimer::singleShot(10, this, [this, spEosRunnerMain, sSceneName = m_sSceneName]
    { SlotRun(spEosRunnerMain, c_sMainRunner, "", sSceneName); });

  return std::make_shared<CEosScriptRunnerInstanceController>(c_sMainRunner, spEosRunnerMain);
}

//----------------------------------------------------------------------------------------
//
void CEosScriptRunner::RegisterNewComponent(const QString& sName,
                                            std::weak_ptr<CScriptCommunicator> wpCommunicator)
{
  QMutexLocker locker(&m_objectMapMutex);
  auto it = m_objectMap.find(sName);
  if (m_objectMap.end() == it)
  {
    if (auto spComm = wpCommunicator.lock())
    {
      std::shared_ptr<CScriptObjectBase> spObject =
          std::shared_ptr<CScriptObjectBase>(
          spComm->CreateNewScriptObject(m_spEosParser.get()));
      if (nullptr != spObject)
      {
        if (spObject->thread() != thread())
        {
          spObject->moveToThread(thread());

          // special handling for notifications: erase runner if overlay was closed pre-maturely
          if (auto spNotification = std::dynamic_pointer_cast<CEosScriptNotification>(spObject))
          {
            connect(spNotification.get(), &CEosScriptNotification::SignalOverlayClosed,
                    this, [this](){
              emit SignalClearThreads(EScriptRunnerType::eOverlay);
            });
            connect(spNotification.get(), &CEosScriptNotification::SignalOverlayRunAsync,
                    this, [this](const QString& sId) {
              auto spRunner =
                  std::dynamic_pointer_cast<CEosScriptRunnerInstanceController>(
                  m_pRunnerParent->RunnerController(sId));
              if (nullptr != spRunner)
              {
                // we would call SignalOverlayRunAsync here normally,
                // but this type of script can only use
                // eos scripts as notifications
                SlotRun(spRunner->Runner(), sId, sId, QString());
              }
            });
          }
        }
        m_objectMap.insert({ sName, spObject });
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEosScriptRunner::UnregisterComponent(const QString& sName)
{
  Q_UNUSED(sName)
}

//----------------------------------------------------------------------------------------
//
void CEosScriptRunner::UnregisterComponents()
{
  m_objectMapMutex.lock();
  for (auto it = m_objectMap.begin(); m_objectMap.end() != it; ++it)
  {
    it->second->Cleanup();
  }
  m_objectMapMutex.unlock();

  m_spEosParser->ClearInstructions();
  m_spEosParser->RegisterInstruction("end", std::make_shared<CCommandEosEnd>());
  m_spEosParser->RegisterInstruction("goto", std::make_shared<CCommandEosGoto>());
  m_spEosParser->RegisterInstruction("noop", std::make_shared<CCommandEosNoop>());

  m_objectMapMutex.lock();
  m_objectMap.clear();
  m_objectMapMutex.unlock();
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<IScriptRunnerInstanceController>
CEosScriptRunner::RunAsync(const QString& sId, const QString& sScript,
                           tspResource spResource)
{
  Q_UNUSED(spResource)
  // create runner
  std::shared_ptr<CJsonInstructionSetRunner> spEosRunner =
      m_spEosParser->ParseJson(sScript);

  if (nullptr == spEosRunner)
  {
    HandleError(m_spEosParser->Error());
    return nullptr;
  }

  spEosRunner->setObjectName(sId);
  connect(spEosRunner.get(), &CJsonInstructionSetRunner::CommandRetVal,
          this, &CEosScriptRunner::SlotCommandRetVal);
  connect(spEosRunner.get(), &CJsonInstructionSetRunner::Fork,
          this, &CEosScriptRunner::SlotFork);

  SlotRun(spEosRunner, sId, "", QString());

  return std::make_shared<CEosScriptRunnerInstanceController>(sId, spEosRunner);
}

//----------------------------------------------------------------------------------------
//
void CEosScriptRunner::HandleScriptFinish(bool bSuccess, const QVariant& sRetVal)
{
  const QString sId = sender()->objectName();

  auto spSignalEmitterContext = m_wpSignalEmitterContext.lock();
  if (nullptr == spSignalEmitterContext)
  {
    HandleError(SJsonException{"Internal error.","","",0,0});
    return;
  }

  emit SignalRemoveScriptRunner(sId);

  if (!bSuccess)
  {
    qWarning() << tr("Error in script or debugger closed, unloading project.");
  }

  if (c_sMainRunner == sId || QVariant::String == sRetVal.type() ||
      !m_pRunnerParent->HasRunningScripts())
  {
    if (QVariant::String == sRetVal.type())
    {
      emit SignalScriptRunFinished(bSuccess, sRetVal.toString());
    }
    else
    {
      emit SignalScriptRunFinished(bSuccess, QString());
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEosScriptRunner::SlotCommandRetVal(CJsonInstructionSetRunner::tRetVal retVal)
{
  if (std::holds_alternative<SJsonException>(retVal))
  {
    HandleError(std::get<SJsonException>(retVal));
  }
  else
  {
    SRunnerRetVal& retValCasted = std::get<SRunnerRetVal>(retVal);
    // retValCasted.m_bHasMoreCommands
    if (retValCasted.m_retVal.type() == typeid(QString))
    {
      HandleScriptFinish(true, std::any_cast<QString>(retValCasted.m_retVal));
    }
    else if (retValCasted.m_retVal.type() == typeid(SFinishTag))
    {
      HandleScriptFinish(true, QVariant());
    }
    else
    {
      HandleScriptFinish(true, QString());
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEosScriptRunner::SlotFork(
    std::shared_ptr<CJsonInstructionSetRunner> spNewRunner, const QString& sForkCommandsName,
    bool bAutoRun)
{
  if (nullptr == spNewRunner)
  {
    HandleError(m_spEosParser->Error());
    return;
  }

  emit SignalAddScriptRunner(sForkCommandsName,
                             std::make_shared<CEosScriptRunnerInstanceController>(
                                 sForkCommandsName, spNewRunner), EScriptRunnerType::eAsync);

  spNewRunner->setObjectName(sForkCommandsName);
  connect(spNewRunner.get(), &CJsonInstructionSetRunner::CommandRetVal,
          this, &CEosScriptRunner::SlotCommandRetVal);
  connect(spNewRunner.get(), &CJsonInstructionSetRunner::Fork,
          this, &CEosScriptRunner::SlotFork);

  if (bAutoRun)
  {
    SlotRun(spNewRunner, sForkCommandsName, sForkCommandsName, QString());
  }
}

//----------------------------------------------------------------------------------------
//
void CEosScriptRunner::SlotRun(std::shared_ptr<CJsonInstructionSetRunner> spEosRunner,
                               const QString& sRunner, const QString& sCommands,
                               const QString sSceneName)
{
  if (1 != m_bInitialized) { return; }

  if (nullptr != spEosRunner)
  {
    if (!sSceneName.isEmpty())
    {
      emit SignalSceneLoaded(sSceneName);
    }

    CJsonInstructionSetRunner::tRetVal retVal =
        spEosRunner->Run(sCommands, ERunerMode::eAutoRunAll, false);
    if (!std::holds_alternative<SRunnerRetVal>(retVal))
    {
      emit SignalRemoveScriptRunner(sRunner);
      HandleError(std::get<SJsonException>(retVal));
    }
  }
  else
  {
    HandleError(SJsonException{"Internal error: Runner unavailable","","",0,0});
  }
}

//----------------------------------------------------------------------------------------
//
void CEosScriptRunner::HandleError(const SJsonException& value)
{
  QString sException = value.m_sException;
  QString sError = "Uncaught exception: " + sException;
  qWarning() << sError;

  auto spSignalEmitterContext = m_wpSignalEmitterContext.lock();
  if (nullptr == spSignalEmitterContext)
  {
    qWarning() << "SignalEmitter is null";
    return;
  }

  emit spSignalEmitterContext->showError(sError, QtMsgType::QtCriticalMsg);
  emit spSignalEmitterContext->executionError(value.m_sException, value.m_iLineNr, sError);
}
