#include "ScriptRunner.h"
#include "Application.h"
#include "DatabaseManager.h"
#include "Project.h"
#include "Resource.h"
#include "Scene.h"

#include "Script/EosScriptRunner.h"
#include "Script/JsScriptRunner.h"
#include "Script/LuaScriptRunner.h"
#include "Script/ScriptRunnerInstanceController.h"
#include "Script/ScriptRunnerSignalEmiter.h"
#include "Script/SequenceRunner.h"

#include <QDebug>
#include <QFileInfo>

//----------------------------------------------------------------------------------------
//
CScriptRunner::CScriptRunner() :
  CSystemBase(),
  m_spRunnerFactoryMap(),
  m_runnerMutex(QMutex::Recursive),
  m_spSignalEmitterContext(nullptr)
{
  qRegisterMetaType<std::shared_ptr<IScriptRunnerInstanceController>>();
}

CScriptRunner::~CScriptRunner()
{

}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<IScriptRunnerInstanceController> CScriptRunner::RunnerController(const QString& sId) const
{
  QMutexLocker locker(&m_runnerMutex);
  auto it = m_vspRunner.find(sId);
  if (m_vspRunner.end() != it)
  {
    return it->second;
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CScriptRunnerSignalContext> CScriptRunner::SignalEmmitterContext() const
{
  return m_spSignalEmitterContext;
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::Initialize()
{
  m_spSignalEmitterContext = std::make_shared<CScriptRunnerSignalContext>();

  auto fnHasRunningScripts = std::bind(&CScriptRunner::HasRunningScripts, this);

  m_spRunnerFactoryMap.insert({SScriptDefinitionData::c_sScriptTypeJs,
                        std::make_unique<CJsScriptRunner>(m_spSignalEmitterContext,
                                                          fnHasRunningScripts)});
  m_spRunnerFactoryMap.insert({SScriptDefinitionData::c_sScriptTypeEos,
                        std::make_unique<CEosScriptRunner>(m_spSignalEmitterContext, this)});
  m_spRunnerFactoryMap.insert({SScriptDefinitionData::c_sScriptTypeLua,
                        std::make_unique<CLuaScriptRunner>(m_spSignalEmitterContext,
                                                           fnHasRunningScripts)});
  m_spRunnerFactoryMap.insert({SScriptDefinitionData::c_sFileTypeSequence,
                               std::make_unique<CSequenceRunner>(m_spSignalEmitterContext,
                                                                 fnHasRunningScripts)});

  m_vspRunner.clear();
  for (const auto& it : m_spRunnerFactoryMap)
  {
    it.second->Initialize();
    bool bOk = connect(dynamic_cast<QObject*>(it.second.get()),
                       SIGNAL(SignalAddScriptRunner(const QString&,std::shared_ptr<IScriptRunnerInstanceController>)),
                       this,
                       SLOT(SlotAddScriptController(const QString&,std::shared_ptr<IScriptRunnerInstanceController>)));
    assert(bOk); Q_UNUSED(bOk);
    bOk = connect(dynamic_cast<QObject*>(it.second.get()), SIGNAL(SignalOverlayCleared()),
                  this, SLOT(SlotOverlayCleared()));
    assert(bOk); Q_UNUSED(bOk);
    bOk = connect(dynamic_cast<QObject*>(it.second.get()), SIGNAL(SignalOverlayClosed(const QString&)),
                  this, SLOT(SlotOverlayClosed(const QString&)));
    assert(bOk); Q_UNUSED(bOk);
    bOk = connect(dynamic_cast<QObject*>(it.second.get()), SIGNAL(SignalOverlayRunAsync(tspProject,const QString&,const QString&)),
                  this, SLOT(SlotOverlayRunAsync(tspProject,const QString&,const QString&)));
    assert(bOk); Q_UNUSED(bOk);
    bOk = connect(dynamic_cast<QObject*>(it.second.get()), SIGNAL(SignalRemoveScriptRunner(const QString&)),
                  this, SLOT(SlotRemoveScriptRunner(const QString&)));
    assert(bOk); Q_UNUSED(bOk);
    bOk = connect(dynamic_cast<QObject*>(it.second.get()), SIGNAL(SignalSceneLoaded(const QString&)),
                  this, SIGNAL(SignalSceneLoaded(const QString&)));
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

  m_vspRunner.clear();
  for (const auto& it : m_spRunnerFactoryMap)
  {
    it.second->Deinitialize();
  }

  m_spSignalEmitterContext = nullptr;

  m_spRunnerFactoryMap.clear();

  SetInitialized(false);
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::LoadScript(tspScene spScene, tspResource spResource)
{
  LoadScriptAndCall(spScene, spResource,
                    [this](std::unique_ptr<IScriptRunnerFactory>& spRunner,
                       const QString& sScript, tspScene spScene, tspResource spResource) {
    QMutexLocker locker(&m_runnerMutex);
    auto it = m_vspRunner.find(IScriptRunnerFactory::c_sMainRunner);
    if (m_vspRunner.end() != it)
    {
      it->second->InterruptExecution();
      it->second->ResetEngine();
      m_vspRunner.erase(it);
    }

    std::shared_ptr<IScriptRunnerInstanceController> spController =
        spRunner->LoadScript(sScript, spScene, spResource);

    if (nullptr != spController)
    {
      m_vspRunner.insert({IScriptRunnerFactory::c_sMainRunner, spController});
    }
  });
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::InterruptExecution()
{
  // interrupt, in case of infinite loop
  m_spSignalEmitterContext->SetScriptExecutionStatus(CScriptRunnerSignalEmiter::eStopped);
  // TODO: check if needed
  if (0 >= m_vspRunner.size())
  {
    emit SignalScriptRunFinished(false, QString());
  }
  for (const auto& it : m_vspRunner)
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

    emit m_spSignalEmitterContext->pauseExecution();
    emit SignalRunningChanged(false);
  }
}

//----------------------------------------------------------------------------------------
//
bool CScriptRunner::HasRunningScripts() const
{
  bool bHasRunningScripts = false;
  for (const auto& it : m_vspRunner)
  {
    bHasRunningScripts |= it.second->IsRunning();
  }
  return bHasRunningScripts;
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::ResumeExecution()
{
  if (CScriptRunnerSignalEmiter::ePaused == m_spSignalEmitterContext->ScriptExecutionStatus() &&
      CScriptRunnerSignalEmiter::eStopped != m_spSignalEmitterContext->ScriptExecutionStatus())
  {
    m_spSignalEmitterContext->SetScriptExecutionStatus(CScriptRunnerSignalEmiter::eRunning);

    emit m_spSignalEmitterContext->resumeExecution();
    emit SignalRunningChanged(true);
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::RegisterNewComponent(const QString sName, QJSValue signalEmitter)
{
  for (const auto& it : m_spRunnerFactoryMap)
  {
    it.second->RegisterNewComponent(sName, signalEmitter);
  }

  CScriptRunnerSignalEmiter* pObject = nullptr;
  if (signalEmitter.isObject())
  {
    pObject = qobject_cast<CScriptRunnerSignalEmiter*>(signalEmitter.toQObject());
  }

  {
    QMutexLocker locker(&m_runnerMutex);
    for (auto& it : m_vspRunner)
    {
      it.second->RegisterNewComponent(sName, pObject);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::UnregisterComponents()
{
  for (const auto& it : m_spRunnerFactoryMap)
  {
    it.second->UnregisterComponents();
  }

  {
    QMutexLocker locker(&m_runnerMutex);
    for (auto& it : m_vspRunner)
    {
      it.second->UnregisterComponents();
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::SlotAddScriptController(const QString& sId,
                                            std::shared_ptr<IScriptRunnerInstanceController> spController)
{
  QMutexLocker locker(&m_runnerMutex);
  auto it = m_vspRunner.find(sId);
  if (m_vspRunner.end() != it)
  {
    it->second->InterruptExecution();
    it->second->ResetEngine();
    m_vspRunner.erase(it);
  }

  if (nullptr != spController)
  {
    m_vspRunner.insert({sId, spController});
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::SlotOverlayCleared()
{
  QMutexLocker locker(&m_runnerMutex);
  auto it = m_vspRunner.begin();
  while (m_vspRunner.end() != it)
  {
    if (IScriptRunnerFactory::c_sMainRunner != it->first)
    {
      it->second->ResetEngine();
      it = m_vspRunner.erase(it);
    }
    else
    {
      ++it;
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::SlotOverlayClosed(const QString& sId)
{
  QMutexLocker locker(&m_runnerMutex);
  auto it = m_vspRunner.find(sId);
  if (m_vspRunner.end() == it)
  {
    return;
  }

  it->second->ResetEngine();

  if (IScriptRunnerFactory::c_sMainRunner != sId)
  {
    m_vspRunner.erase(it);
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
                      [this, sId](std::unique_ptr<IScriptRunnerFactory>& spRunner,
                         const QString& sScript, tspScene, tspResource spResource) {

      QMutexLocker locker(&m_runnerMutex);
      auto it = m_vspRunner.find(sId);
      if (m_vspRunner.end() != it)
      {
        it->second->InterruptExecution();
        it->second->ResetEngine();
        m_vspRunner.erase(it);
      }

      std::shared_ptr<IScriptRunnerInstanceController> spController =
          spRunner->OverlayRunAsync(sId, sScript, spResource);

      if (nullptr != spController)
      {
        m_vspRunner.insert({sId, spController});
      }

    });
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::SlotRemoveScriptRunner(const QString& sId)
{
  QMutexLocker locker(&m_runnerMutex);
  auto it = m_vspRunner.find(sId);
  if (m_vspRunner.end() == it)
  {
    qWarning() << QString("Script runner %1 not found").arg(sId);
    return;
  }

  it->second->ResetEngine();

  m_vspRunner.erase(it);
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::SlotScriptRunFinished(bool bOk, const QString& sRetVal)
{
  if (!HasRunningScripts())
  {
    m_spSignalEmitterContext->SetScriptExecutionStatus(CScriptRunnerSignalEmiter::eStopped);
  }

  emit SignalScriptRunFinished(bOk, sRetVal);
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::LoadScriptAndCall(tspScene spScene, tspResource spResource,
                                      std::function<void(std::unique_ptr<IScriptRunnerFactory>&,
                                                         const QString&, tspScene, tspResource)> fn)
{
  if (nullptr == m_spSignalEmitterContext)
  {
    qWarning() << "m_wpSignalEmitterContext was null";
    return;
  }

  if (nullptr == spResource ||
      nullptr == spResource->m_spParent)
  {
    QString sError = tr("Script file, or Project is null");
    qCritical() << sError;
    emit m_spSignalEmitterContext->showError(sError, QtMsgType::QtCriticalMsg);
    return;
  }

  tspProject spProject;
  QReadLocker projectLocker(&spResource->m_spParent->m_rwLock);
  QReadLocker resourceLocker(&spResource->m_rwLock);
  spProject = spResource->m_spParent;
  QUrl sResourceUrl = spResource->m_sPath;
  if (spResource->m_type._to_integral() != EResourceType::eScript ||
      spResource->m_type._to_integral() != EResourceType::eSequence)
  {
    QString sError = tr("Resource is of wrong type: Script or Sequence type required.");
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
      auto it = m_spRunnerFactoryMap.find(sSuffix);
      if (m_spRunnerFactoryMap.end() != it)
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
