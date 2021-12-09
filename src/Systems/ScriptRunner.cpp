#include "ScriptRunner.h"
#include "Project.h"
#include "Resource.h"
#include "Scene.h"
#include "Script/EosScriptRunner.h"
#include "Script/JsScriptRunner.h"
#include "Script/ScriptRunnerSignalEmiter.h"
#include <QDebug>
#include <QFileInfo>

//----------------------------------------------------------------------------------------
//
CScriptRunner::CScriptRunner() :
  CSystemBase(),
  m_spRunnerMap(),
  m_spSignalEmitterContext(nullptr)
{
}

CScriptRunner::~CScriptRunner()
{

}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CScriptRunnerSignalContext> CScriptRunner::SignalEmmitterContext()
{
  return m_spSignalEmitterContext;
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::Initialize()
{
  m_spSignalEmitterContext = std::make_shared<CScriptRunnerSignalContext>();

  m_spRunnerMap.insert({"js",
                        std::make_unique<CJsScriptRunner>(m_spSignalEmitterContext)});
  m_spRunnerMap.insert({"eos",
                        std::make_unique<CEosScriptRunner>(m_spSignalEmitterContext)});

  for (const auto& it : m_spRunnerMap)
  {
    it.second->Initialize();
    bool bOk = connect(dynamic_cast<QObject*>(it.second.get()), SIGNAL(SignalScriptRunFinished(bool,const QString&)),
                       this, SLOT(SlotScriptRunFinished(bool,const QString&)));
    assert(bOk); Q_UNUSED(bOk);
  }

  SetInitialized(true);
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::Deinitialize()
{
  InterruptExecution();
  UnregisterComponents();

  for (const auto& it : m_spRunnerMap)
  {
    it.second->Deinitialize();
  }

  m_spSignalEmitterContext = nullptr;

  m_spRunnerMap.clear();

  SetInitialized(false);
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::LoadScript(tspScene spScene, tspResource spResource)
{
  if (nullptr == spResource || nullptr == spScene ||
      nullptr == spResource->m_spParent)
  {
    QString sError = tr("Script file, Scene or Project is null");
    qCritical() << sError;
    emit m_spSignalEmitterContext->showError(sError, QtMsgType::QtCriticalMsg);
    return;
  }

  tspProject spProject;
  QReadLocker projectLocker(&spResource->m_spParent->m_rwLock);
  QReadLocker resourceLocker(&spResource->m_rwLock);
  spProject = spResource->m_spParent;
  if (spResource->m_type._to_integral() != EResourceType::eScript)
  {
    QString sError = tr("Script resource is of wrong type.");
    qCritical() << sError;
    emit m_spSignalEmitterContext->showError(sError, QtMsgType::QtCriticalMsg);
    return;
  }

  resourceLocker.unlock();
  projectLocker.unlock();
  QString sPath = ResourceUrlToAbsolutePath(spResource);

  QFileInfo scriptFileInfo(sPath);
  if (scriptFileInfo.exists())
  {
    QFile scriptFile(sPath);
    if (scriptFile.open(QIODevice::ReadOnly))
    {
      QString sScript = QString::fromUtf8(scriptFile.readAll());
      const QString sSuffix = scriptFileInfo.suffix();
      auto it = m_spRunnerMap.find(sSuffix);
      if (m_spRunnerMap.end() != it)
      {
        it->second->LoadScript(sScript, spScene, spResource);
      }
    }
    else
    {
      QString sError = tr("Script resource file could not be opened.");
      qCritical() << sError;
      emit m_spSignalEmitterContext->showError(sError, QtMsgType::QtCriticalMsg);
      return;
    }
  }
  else
  {
    QString sError = tr("Script resource file does not exist.");
    qCritical() << sError;
    emit m_spSignalEmitterContext->showError(sError, QtMsgType::QtCriticalMsg);
    return;
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::InterruptExecution()
{
  // interrupt, in case of infinite loop
  m_spSignalEmitterContext->SetScriptExecutionStatus(CScriptRunnerSignalEmiter::eStopped);
  for (const auto& it : m_spRunnerMap)
  {
    it.second->InterruptExecution();
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::PauseExecution()
{
  if (CScriptRunnerSignalEmiter::ePaused != m_spSignalEmitterContext->ScriptExecutionStatus() &&
      CScriptRunnerSignalEmiter::eStopped != m_spSignalEmitterContext->ScriptExecutionStatus())
  {
    m_spSignalEmitterContext->SetScriptExecutionStatus(CScriptRunnerSignalEmiter::ePaused);
    for (const auto& it : m_spRunnerMap)
    {
      it.second->PauseExecution();
    }
    emit m_spSignalEmitterContext->pauseExecution();
    emit SignalRunningChanged(false);
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::ResumeExecution()
{
  if (CScriptRunnerSignalEmiter::ePaused == m_spSignalEmitterContext->ScriptExecutionStatus() &&
      CScriptRunnerSignalEmiter::eStopped != m_spSignalEmitterContext->ScriptExecutionStatus())
  {
    m_spSignalEmitterContext->SetScriptExecutionStatus(CScriptRunnerSignalEmiter::eRunning);
    for (const auto& it : m_spRunnerMap)
    {
      it.second->ResumeExecution();
    }
    emit m_spSignalEmitterContext->resumeExecution();
    emit SignalRunningChanged(true);
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::RegisterNewComponent(const QString sName, QJSValue signalEmitter)
{
  for (const auto& it : m_spRunnerMap)
  {
    it.second->RegisterNewComponent(sName, signalEmitter);
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::UnregisterComponents()
{
  for (const auto& it : m_spRunnerMap)
  {
    it.second->UnregisterComponents();
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::SlotScriptRunFinished(bool bOk, const QString& sRetVal)
{
  bool bHasRunningScripts = false;
  for (const auto& it : m_spRunnerMap)
  {
    bHasRunningScripts |= it.second->HasRunningScripts();
  }

  if (!bHasRunningScripts)
  {
    m_spSignalEmitterContext->SetScriptExecutionStatus(CScriptRunnerSignalEmiter::eStopped);
  }

  emit SignalScriptRunFinished(bOk, sRetVal);
}

//----------------------------------------------------------------------------------------
//
CScriptRunnerWrapper::CScriptRunnerWrapper(QObject* pParent, std::weak_ptr<CScriptRunner> wpRunner) :
  QObject(pParent),
  m_wpRunner(wpRunner)
{
  connect(wpRunner.lock().get(), &CScriptRunner::SignalRunningChanged,
          this, &CScriptRunnerWrapper::runningChanged, Qt::QueuedConnection);
}
CScriptRunnerWrapper::~CScriptRunnerWrapper()
{

}

//----------------------------------------------------------------------------------------
//
void CScriptRunnerWrapper::pauseExecution()
{
  if (auto spRunner = m_wpRunner.lock())
  {
    spRunner->PauseExecution();
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptRunnerWrapper::registerNewComponent(const QString sName, QJSValue signalEmitter)
{
  if (auto spRunner = m_wpRunner.lock())
  {
    spRunner->RegisterNewComponent(sName, signalEmitter);
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptRunnerWrapper::resumeExecution()
{
  if (auto spRunner = m_wpRunner.lock())
  {
    spRunner->ResumeExecution();
  }
}
