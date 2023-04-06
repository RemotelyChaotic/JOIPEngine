#include "ScriptRunner.h"
#include "Application.h"
#include "DatabaseManager.h"
#include "Project.h"
#include "Resource.h"
#include "Scene.h"

#include "Script/EosScriptRunner.h"
#include "Script/JsScriptRunner.h"
#include "Script/LuaScriptRunner.h"
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
  m_spRunnerMap.insert({"lua",
                        std::make_unique<CLuaScriptRunner>(m_spSignalEmitterContext)});

  for (const auto& it : m_spRunnerMap)
  {
    it.second->Initialize();
    bool bOk = connect(dynamic_cast<QObject*>(it.second.get()), SIGNAL(SignalOverlayCleared()),
                       this, SLOT(SlotOverlayCleared()));
    assert(bOk); Q_UNUSED(bOk);
    bOk = connect(dynamic_cast<QObject*>(it.second.get()), SIGNAL(SignalOverlayClosed(const QString&)),
                  this, SLOT(SlotOverlayClosed(const QString&)));
    assert(bOk); Q_UNUSED(bOk);
    bOk = connect(dynamic_cast<QObject*>(it.second.get()), SIGNAL(SignalOverlayRunAsync(tspProject,const QString&,const QString&)),
                  this, SLOT(SlotOverlayRunAsync(tspProject,const QString&,const QString&)));
    assert(bOk); Q_UNUSED(bOk);
    bOk = connect(dynamic_cast<QObject*>(it.second.get()), SIGNAL(SignalScriptRunFinished(bool,const QString&)),
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
  LoadScriptAndCall(spScene, spResource,
                    [](std::unique_ptr<IScriptRunner>& spRunner,
                       const QString& sScript, tspScene spScene, tspResource spResource) {
    spRunner->LoadScript(sScript, spScene, spResource);
  });
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
void CScriptRunner::SlotOverlayCleared()
{
  for (const auto& it : m_spRunnerMap)
  {
    it.second->OverlayCleared();
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::SlotOverlayClosed(const QString& sId)
{
  for (const auto& it : m_spRunnerMap)
  {
    it.second->OverlayClosed(sId);
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::SlotOverlayRunAsync(tspProject spProject, const QString& sId,
                                        const QString& sScriptResource)
{
  if (auto spDbManager = CApplication::Instance()->System<CDatabaseManager>().lock())
  {
    tspResource spResource = spDbManager->FindResourceInProject(spProject, sScriptResource);

    LoadScriptAndCall(nullptr, spResource,
                      [sId](std::unique_ptr<IScriptRunner>& spRunner,
                         const QString& sScript, tspScene, tspResource spResource) {
      spRunner->OverlayRunAsync(sId, sScript, spResource);
    });
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
void CScriptRunner::LoadScriptAndCall(tspScene spScene, tspResource spResource,
                                      std::function<void(std::unique_ptr<IScriptRunner>&,
                                                         const QString&, tspScene, tspResource)> fn)
{
  if (nullptr == m_spSignalEmitterContext)
  {
    qWarning() << "m_wpSignalEmitterContext was null";
    return;
  }

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
  QUrl sResourceUrl = spResource->m_sPath;
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
      // get suffix from the path, as bundles might not have the suffix in the name
      const QString sSuffix = QFileInfo(sResourceUrl.toString()).suffix();
      auto it = m_spRunnerMap.find(sSuffix);
      if (m_spRunnerMap.end() != it)
      {
        fn(it->second, sScript, spScene, spResource);
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
