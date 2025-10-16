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
    return it->second.second;
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
                       SIGNAL(SignalAddScriptRunner(const QString&,std::shared_ptr<IScriptRunnerInstanceController>,EScriptRunnerType)),
                       this,
                       SLOT(SlotAddScriptController(const QString&,std::shared_ptr<IScriptRunnerInstanceController>,EScriptRunnerType)));
    assert(bOk); Q_UNUSED(bOk);
    bOk = connect(dynamic_cast<QObject*>(it.second.get()), SIGNAL(SignalClearThreads(EScriptRunnerType)),
                  this, SLOT(SlotClearThreads(EScriptRunnerType)));
    assert(bOk); Q_UNUSED(bOk);
    bOk = connect(dynamic_cast<QObject*>(it.second.get()), SIGNAL(SignalKill(const QString&)),
                  this, SLOT(SlotKill(const QString&)));
    assert(bOk); Q_UNUSED(bOk);
    bOk = connect(dynamic_cast<QObject*>(it.second.get()), SIGNAL(SignalRunAsync(tspProject,const QString&,const QString&,EScriptRunnerType)),
                  this, SLOT(SlotSignalRunAsync(tspProject,const QString&,const QString&,EScriptRunnerType)));
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
    auto it = std::find_if(m_vspRunner.begin(), m_vspRunner.end(),
                           [](const std::pair<QString, std::pair<EScriptRunnerType,
                                                                 std::shared_ptr<IScriptRunnerInstanceController>>>& pair) {
      return pair.second.first == EScriptRunnerType::eMain;
    });
    if (m_vspRunner.end() != it)
    {
      it->second.second->InterruptExecution();
      it->second.second->ResetEngine();
      m_vspRunner.erase(it);
    }

    std::shared_ptr<IScriptRunnerInstanceController> spController =
        spRunner->LoadScript(sScript, spScene, spResource);

    if (nullptr != spController)
    {
      m_vspRunner.insert({IScriptRunnerFactory::c_sMainRunner,
                          {EScriptRunnerType::eMain, spController}});
    }
  });
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::InterruptExecution()
{
  // interrupt, in case of infinite loop
  m_spSignalEmitterContext->SetScriptExecutionStatus(CScriptRunnerSignalEmiter::eStopped);

  QMutexLocker locker(&m_runnerMutex);
  if (0 >= m_vspRunner.size())
  {
    emit SignalScriptRunFinished(false, QString());
  }
  for (const auto& it : m_vspRunner)
  {
    it.second.second->InterruptExecution();
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
    bHasRunningScripts |= it.second.second->IsRunning();
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
  if (QThread::currentThread() != qApp->thread())
  {
    assert(false && "Called RegisterNewComponent in wrong thread.");
    qWarning() << tr("Called RegisterNewComponent in wrong thread.");
    return;
  }

  CScriptRunnerSignalEmiter* pObject = nullptr;
  if (signalEmitter.isObject())
  {
    pObject = qobject_cast<CScriptRunnerSignalEmiter*>(signalEmitter.toQObject());
  }

  if (nullptr == pObject)
  {
    assert(false && "SignalEmitter is not a QObject");
    qWarning() << tr("SignalEmitter is not a QObject");
    return;
  }

  for (const auto& it : m_spRunnerFactoryMap)
  {
    it.second->RegisterNewComponent(sName, signalEmitter);
  }

  {
    QMutexLocker locker(&m_runnerMutex);
    for (auto& it : m_vspRunner)
    {
      it.second.second->RegisterNewComponent(sName, pObject);
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
      it.second.second->UnregisterComponents();
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::SlotAddScriptController(const QString& sId,
                                            std::shared_ptr<IScriptRunnerInstanceController> spController,
                                            EScriptRunnerType type)
{
  QMutexLocker locker(&m_runnerMutex);
  auto it = m_vspRunner.find(sId);
  if (m_vspRunner.end() != it)
  {
    it->second.second->InterruptExecution();
    it->second.second->ResetEngine();
    m_vspRunner.erase(it);
  }

  if (nullptr != spController)
  {
    m_vspRunner.insert({sId, {type, spController}});
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::SlotClearThreads(EScriptRunnerType type)
{
  QMutexLocker locker(&m_runnerMutex);
  for (auto it = m_vspRunner.begin(); m_vspRunner.end() != it;)
  {
    if (type == it->second.first)
    {
      it->second.second->InterruptExecution();
      it->second.second->ResetEngine();
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
void CScriptRunner::SlotKill(const QString& sId)
{
  QMutexLocker locker(&m_runnerMutex);
  auto it = m_vspRunner.find(sId);
  if (m_vspRunner.end() == it)
  {
    return;
  }

  it->second.second->InterruptExecution();
  it->second.second->ResetEngine();

  if (EScriptRunnerType::eMain != it->second.first)
  {
    m_vspRunner.erase(it);
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::SlotSignalRunAsync(tspProject spProject, const QString& sId,
                                       const QString& sScriptResource,
                                       EScriptRunnerType type)
{
  if (auto spDbManager = CApplication::Instance()->System<CDatabaseManager>().lock())
  {
    tspResource spResource = spDbManager->FindResourceInProject(spProject, sScriptResource);

    LoadScriptAndCall(nullptr, spResource,
                      [this, sId, type](std::unique_ptr<IScriptRunnerFactory>& spRunner,
                         const QString& sScript, tspScene, tspResource spResource) {

      std::shared_ptr<IScriptRunnerInstanceController> spController =
          spRunner->RunAsync(sId, sScript, spResource);

      SlotAddScriptController(sId, spController, type);
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

  it->second.second->InterruptExecution();
  it->second.second->ResetEngine();

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
  if (spResource->m_type._to_integral() != EResourceType::eScript &&
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
