#include "EosScriptRunner.h"
#include "ScriptObjectBase.h"
#include "ScriptRunnerSignalEmiter.h"
#include "Systems/JSON/JsonInstructionSetParser.h"
#include "Systems/JSON/JsonInstructionSetRunner.h"
#include "Systems/Resource.h"
#include "Systems/Scene.h"

#include <QDebug>
#include <QFile>

CEosScriptRunner::CEosScriptRunner(std::weak_ptr<CScriptRunnerSignalContext> spSignalEmitterContext,
                                   QObject* pParent) :
  QObject(pParent),
  IScriptRunner(),
  m_spEosParser(std::make_unique<CJsonInstructionSetParser>()),
  m_spEosRunner(nullptr),
  m_spTimer(std::make_shared<QTimer>()),
  m_wpSignalEmitterContext(spSignalEmitterContext),
  m_spCurrentScene(nullptr),
  m_sceneMutex(QMutex::Recursive),
  m_sSceneName(),
  m_objectMapMutex(QMutex::Recursive),
  m_objectMap()
{
  connect(m_spTimer.get(), &QTimer::timeout, this, &CEosScriptRunner::SlotRun);
}

CEosScriptRunner::~CEosScriptRunner()
{}

//----------------------------------------------------------------------------------------
//
void CEosScriptRunner::Initialize()
{
  m_spEosParser->RegisterInstructionSetPath("Commands", "/");
}

//----------------------------------------------------------------------------------------
//
void CEosScriptRunner::Deinitialize()
{
  m_spTimer->stop();
  m_spEosRunner = nullptr;
}

//----------------------------------------------------------------------------------------
//
void CEosScriptRunner::InterruptExecution()
{
  // TODO:
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
void CEosScriptRunner::LoadScript(const QString& sScript, tspScene spScene, tspResource spResource)
{
  // set scene
  m_spCurrentScene = spScene;
  //

  // get scene name
  QReadLocker locker(&spScene->m_rwLock);
  QMutexLocker lockerScene(&m_sceneMutex);
  m_sSceneName = spScene->m_sName;
  lockerScene.unlock();
  locker.unlock();

  // set current Project
  m_objectMapMutex.lock();
  for (auto it = m_objectMap.begin(); m_objectMap.end() != it; ++it)
  {
    it->second->SetCurrentProject(spResource->m_spParent);
  }
  m_objectMapMutex.unlock();

  auto spSignalEmitterContext = m_wpSignalEmitterContext.lock();
  if (nullptr == spSignalEmitterContext)
  {
    HandleError(SJsonException{"Internal error.","","",0,0});
    return;
  }

  // create runner
  m_spEosRunner =
    m_spEosParser->ParseJson(sScript);
  if (nullptr == m_spEosParser)
  {
    HandleError(m_spEosParser->Error());
    return;
  }

  spSignalEmitterContext->SetScriptExecutionStatus(CScriptRunnerSignalEmiter::eRunning);

  m_spTimer->setSingleShot(true);
  m_spTimer->start(10);
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
      if (spObject->thread() != thread())
      {
        spObject->moveToThread(thread());
      }
      m_objectMap.insert({ sName, spObject });
      QMetaObject::invokeMethod(this, "SlotRegisterObject", Qt::QueuedConnection,
                                Q_ARG(QString, sName));
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

  m_spCurrentScene = nullptr;

  m_objectMapMutex.lock();
  m_objectMap.clear();
  m_objectMapMutex.unlock();
}

//----------------------------------------------------------------------------------------
//
void CEosScriptRunner::HandleScriptFinish(bool bSuccess, const QVariant& sRetVal)
{
  auto spSignalEmitterContext = m_wpSignalEmitterContext.lock();
  if (nullptr == spSignalEmitterContext)
  {
    HandleError(SJsonException{"Internal error.","","",0,0});
    return;
  }

  spSignalEmitterContext->SetScriptExecutionStatus(CScriptRunnerSignalEmiter::eStopped);

  m_spCurrentScene = nullptr;

  emit spSignalEmitterContext->interrupt();

  m_objectMapMutex.lock();
  for (auto it = m_objectMap.begin(); m_objectMap.end() != it; ++it)
  {
    it->second->SetCurrentProject(nullptr);
  }
  m_objectMapMutex.unlock();

  if (QVariant::String == sRetVal.type())
  {
    emit SignalScriptRunFinished(bSuccess, sRetVal.toString());
  }
  else
  {
    emit SignalScriptRunFinished(bSuccess, QString());
  }
}

//----------------------------------------------------------------------------------------
//
void CEosScriptRunner::SlotRegisterObject(const QString& sObject)
{
  QMutexLocker locker(&m_objectMapMutex);
  auto it = m_objectMap.find(sObject);
  if (m_objectMap.end() != it)
  {
    //m_spEosParser->RegisterInstruction(sObject, it->second);
  }
}

//----------------------------------------------------------------------------------------
//
void CEosScriptRunner::SlotRun()
{
  auto spSignalEmitterContext = m_wpSignalEmitterContext.lock();
  if (nullptr == spSignalEmitterContext)
  {
    HandleError(SJsonException{"Internal error.","","",0,0});
    return;
  }

  CJsonInstructionSetRunner::tRetVal bOk = m_spEosRunner->Run("Commands", ERunerMode::eAutoRunAll);
  spSignalEmitterContext->SetScriptExecutionStatus(CScriptRunnerSignalEmiter::eStopped);

  if (!std::holds_alternative<bool>(bOk))
  {
    HandleError(std::get<SJsonException>(bOk));
  }
  else
  {
    HandleScriptFinish(true, QString());
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
