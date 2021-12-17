#include "EosScriptRunner.h"
#include "ScriptNotification.h"
#include "ScriptObjectBase.h"
#include "ScriptRunnerSignalEmiter.h"

#include "Systems/JSON/JsonInstructionSetParser.h"
#include "Systems/JSON/JsonInstructionSetRunner.h"
#include "Systems/Resource.h"
#include "Systems/Scene.h"

#include <QDebug>
#include <QFile>
#include <QTimer>

namespace  {
  const char* c_sMainRunner = "~main";
}
struct SFinishTag {};

//----------------------------------------------------------------------------------------
//
class CCommandEosEnd : public IJsonInstructionBase
{
public:
  CCommandEosEnd() : m_argTypes() {}
  ~CCommandEosEnd() override {}

  tInstructionMapType& ArgList() override
  {
    return m_argTypes;
  }

  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) override
  {
    Q_UNUSED(args)
    return SRunRetVal<ENextCommandToCall::eFinish>(SFinishTag());
  }

private:
  tInstructionMapType    m_argTypes;
};

//----------------------------------------------------------------------------------------
//
class CCommandEosGoto : public IJsonInstructionBase
{
public:
  CCommandEosGoto() :
    m_argTypes({
    { "target", SInstructionArgumentType{EArgumentType::eString}}
  }) {}
  ~CCommandEosGoto() override {}

  tInstructionMapType& ArgList() override
  {
    return m_argTypes;
  }

  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) override
  {
    const auto& itTarget = GetValue<EArgumentType::eString>(args, "target");
    if (HasValue(args, "target") && IsOk<EArgumentType::eString>(itTarget))
    {
      QString sTarget = std::get<QString>(itTarget);
      sTarget.replace("*", "(.*)");
      return SRunRetVal<ENextCommandToCall::eFinish>(sTarget);
    }
    return SJsonException{"internal Error: No target for goto.", "target", "goto", 0, 0};
  }

private:
  tInstructionMapType    m_argTypes;
};

//----------------------------------------------------------------------------------------
//
class CCommandEosNoop : public IJsonInstructionBase
{
public:
  CCommandEosNoop() :
    m_argTypes() {}
  ~CCommandEosNoop() override {}

  tInstructionMapType& ArgList() override
  {
    return m_argTypes;
  }

  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) override
  {
    Q_UNUSED(args)
    return SRunRetVal<ENextCommandToCall::eSibling>();
  }

private:
  tInstructionMapType    m_argTypes;
};

//----------------------------------------------------------------------------------------
//
CEosScriptRunner::CEosScriptRunner(std::weak_ptr<CScriptRunnerSignalContext> spSignalEmitterContext,
                                   QObject* pParent) :
  QObject(pParent),
  IScriptRunner(),
  m_spEosParser(std::make_unique<CJsonInstructionSetParser>()),
  m_vspEosRunner(),
  m_runnerMutex(QMutex::Recursive),
  m_wpSignalEmitterContext(spSignalEmitterContext),
  m_spCurrentScene(nullptr),
  m_sceneMutex(QMutex::Recursive),
  m_sSceneName(),
  m_objectMapMutex(QMutex::Recursive),
  m_objectMap(),
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
  m_spEosParser->RegisterInstruction("end", std::make_shared<CCommandEosEnd>());
  m_spEosParser->RegisterInstruction("goto", std::make_shared<CCommandEosGoto>());
  m_spEosParser->RegisterInstruction("noop", std::make_shared<CCommandEosNoop>());
  m_bInitialized = 1;
}

//----------------------------------------------------------------------------------------
//
void CEosScriptRunner::Deinitialize()
{
  m_bInitialized = 0;
  QMutexLocker locker(&m_runnerMutex);
  m_vspEosRunner.clear();
}

//----------------------------------------------------------------------------------------
//
void CEosScriptRunner::InterruptExecution()
{
  QMutexLocker locker(&m_runnerMutex);
  if (0 >= m_vspEosRunner.size())
  {
    emit SignalScriptRunFinished(false, QString());
  }
  for (auto& it : m_vspEosRunner)
  {
    if (nullptr != it.second)
    {
      it.second->Interrupt();
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEosScriptRunner::PauseExecution()
{

}

//----------------------------------------------------------------------------------------
//
void CEosScriptRunner::ResumeExecution()
{

}

//----------------------------------------------------------------------------------------
//
bool CEosScriptRunner::HasRunningScripts() const
{
  bool bHasRunningScripts = false;
  QMutexLocker locker(&m_runnerMutex);
  for (auto& it : m_vspEosRunner)
  {
    if (nullptr != it.second)
    {
      bHasRunningScripts |= it.second->IsRunning();
    }
  }
  return bHasRunningScripts;
}

//----------------------------------------------------------------------------------------
//
void CEosScriptRunner::LoadScript(const QString& sScript, tspScene spScene, tspResource spResource)
{
  // set scene
  m_spCurrentScene = spScene;
  //

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
    return;
  }

  // create runner
  std::shared_ptr<CJsonInstructionSetRunner> spEosRunnerMain = nullptr;
  {
    QMutexLocker locker(&m_runnerMutex);
    m_vspEosRunner[c_sMainRunner] =
      m_spEosParser->ParseJson(sScript);
    spEosRunnerMain = m_vspEosRunner[c_sMainRunner];
  }

  if (nullptr == spEosRunnerMain)
  {
    HandleError(m_spEosParser->Error());
    return;
  }

  spEosRunnerMain->setObjectName(c_sMainRunner);
  connect(spEosRunnerMain.get(), &CJsonInstructionSetRunner::CommandRetVal,
          this, &CEosScriptRunner::SlotCommandRetVal);
  connect(spEosRunnerMain.get(), &CJsonInstructionSetRunner::Fork,
          this, &CEosScriptRunner::SlotFork);

  spSignalEmitterContext->SetScriptExecutionStatus(CScriptRunnerSignalEmiter::eRunning);

  QTimer::singleShot(10, this, [this]{ SlotRun(c_sMainRunner, ""); });
}

//----------------------------------------------------------------------------------------
//
void CEosScriptRunner::RegisterNewComponent(const QString sName, QJSValue signalEmitter)
{
  QMutexLocker locker(&m_objectMapMutex);
  auto it = m_objectMap.find(sName);
  if (m_objectMap.end() == it)
  {
    if (signalEmitter.isObject())
    {
      CScriptRunnerSignalEmiter* pObject =
          qobject_cast<CScriptRunnerSignalEmiter*>(signalEmitter.toQObject());
      pObject->Initialize(m_wpSignalEmitterContext.lock());
      std::shared_ptr<CScriptObjectBase> spObject =
          pObject->CreateNewScriptObject(m_spEosParser.get());
      if (nullptr != spObject)
      {
        if (spObject->thread() != thread())
        {
          spObject->moveToThread(thread());

          // special handling for notifications: erase runner if overlay was closed pre-maturely
          if (auto spNotification = std::dynamic_pointer_cast<CEosScriptNotification>(spObject))
          {
            connect(spNotification.get(), &CEosScriptNotification::SignalOverlayClosed,
                    this, [this](const QString& sId) {
              QMutexLocker locker(&m_runnerMutex);
              auto it = m_vspEosRunner.find(sId);
              if (m_vspEosRunner.end() != it)
              {
                it->second->Interrupt();
                m_vspEosRunner.erase(it);
              }
            });
            connect(spNotification.get(), &CEosScriptNotification::SignalOverlayRunAsync,
                    this, [this](const QString& sId) {
              QMutexLocker locker(&m_runnerMutex);
              auto it = m_vspEosRunner.find(sId);
              if (m_vspEosRunner.end() != it)
              {
                SlotRun(sId, sId);
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
void CEosScriptRunner::UnregisterComponents()
{
  //m_spScriptEngine->globalObject().setProperty("scene", QJSValue());

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

  m_spCurrentScene = nullptr;

  m_objectMapMutex.lock();
  m_objectMap.clear();
  m_objectMapMutex.unlock();
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

  m_spCurrentScene = nullptr;

  {
    QMutexLocker locker(&m_runnerMutex);
    auto itRunner = m_vspEosRunner.find(sId);
    if (m_vspEosRunner.end() == itRunner || nullptr == itRunner->second)
    {
      qWarning() << tr("Internal error: runner was null.");
      emit SignalScriptRunFinished(false, QString());
      return;
    }
    itRunner->second->Interrupt();
    m_vspEosRunner.erase(itRunner);
  }

  if (!bSuccess)
  {
    qWarning() << tr("Error in script, unloading project.");
    emit SignalScriptRunFinished(false, QString());
  }
  else
  {
    if (QVariant::String == sRetVal.type() && !sRetVal.toString().isEmpty())
    {
      emit SignalScriptRunFinished(bSuccess, sRetVal.toString());
    }
    else if (QVariant::String != sRetVal.type() && sRetVal.isNull())
    {
      emit SignalScriptRunFinished(false, QString());
    }
    else if (spSignalEmitterContext->ScriptExecutionStatus() ==
             CScriptRunnerSignalEmiter::ScriptExecStatus::eStopped)
    {
      emit SignalScriptRunFinished(false, QString());
    }
    // eos does not autorun the next scene
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

  // create runner
  std::shared_ptr<CJsonInstructionSetRunner> spEosRunner = nullptr;
  {
    QMutexLocker locker(&m_runnerMutex);
    m_vspEosRunner[sForkCommandsName] = spNewRunner;
    spEosRunner = m_vspEosRunner[sForkCommandsName];
  }

  spEosRunner->setObjectName(sForkCommandsName);
  connect(spEosRunner.get(), &CJsonInstructionSetRunner::CommandRetVal,
          this, &CEosScriptRunner::SlotCommandRetVal);
  connect(spEosRunner.get(), &CJsonInstructionSetRunner::Fork,
          this, &CEosScriptRunner::SlotFork);

  if (bAutoRun)
  {
    SlotRun(sForkCommandsName, sForkCommandsName);
  }
}

//----------------------------------------------------------------------------------------
//
void CEosScriptRunner::SlotRun(const QString& sRunner, const QString& sCommands)
{
  if (1 != m_bInitialized) { return; }

  QMutexLocker locker(&m_runnerMutex);
  auto it = m_vspEosRunner.find(sRunner);
  if (m_vspEosRunner.end() != it)
  {
    std::shared_ptr<CJsonInstructionSetRunner> spRunner = it->second;
    locker.unlock();

    CJsonInstructionSetRunner::tRetVal retVal = spRunner->Run(sCommands, ERunerMode::eAutoRunAll, false);
    if (!std::holds_alternative<SRunnerRetVal>(retVal))
    {
      locker.relock();
      auto it = m_vspEosRunner.find(sRunner);
      if (m_vspEosRunner.end() != it)
      {
        m_vspEosRunner.erase(it);
      }
      locker.unlock();
      HandleError(std::get<SJsonException>(retVal));
    }
  }
  else
  {
    locker.unlock();
    HandleError(SJsonException{"Internal error: Runner unavailable","","",0,0});
  }
}

//----------------------------------------------------------------------------------------
//
void CEosScriptRunner::HandleError(const SJsonException& value)
{
  QString sException = value.m_sException;
  QString sError = "Uncaught exception: " + sException;
  qCritical() << sError;

  auto spSignalEmitterContext = m_wpSignalEmitterContext.lock();
  if (nullptr == spSignalEmitterContext)
  {
    qCritical() << "SignalEmitter is null";
    return;
  }

  emit spSignalEmitterContext->showError(sError, QtMsgType::QtCriticalMsg);
  emit spSignalEmitterContext->executionError(value.m_sException, value.m_iLineNr, sError);
}
