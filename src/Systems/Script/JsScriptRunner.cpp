#include "JsScriptRunner.h"
#include "Application.h"
#include "ScriptNotification.h"
#include "ScriptRunnerSignalEmiter.h"
#include "ScriptTextBox.h"
#include "Systems/Project.h"
#include "Systems/Resource.h"
#include "Systems/Scene.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QQmlEngine>
#include <QTimer>

namespace  {
  const char* c_sMainRunner = "~main";
}

//----------------------------------------------------------------------------------------
//
class CJsScriptRunnerInstanceWorker : public QObject
{
  Q_OBJECT
public:
  CJsScriptRunnerInstanceWorker(std::weak_ptr<CScriptRunnerSignalContext> wpSignalEmitterContext) :
    QObject(),
    m_wpSignalEmiterContext(wpSignalEmitterContext),
    m_pScriptEngine(nullptr),
    m_pScriptUtils(nullptr),
    m_pCurrentScene(nullptr),
    m_objectMap()
  {}
  ~CJsScriptRunnerInstanceWorker()
  {
  }

public slots:
  //----------------------------------------------------------------------------------------
  //
  void FinishedScript(const QVariant& sRetVal)
  {
    emit HandleScriptFinish(true, sRetVal);
  }

  //----------------------------------------------------------------------------------------
  //
  void HandleError(QJSValue& value)
  {
    auto spSignalEmitterContext = m_wpSignalEmiterContext.lock();
    if (nullptr == spSignalEmitterContext)
    {
      qCritical() << "SignalEmitter is null";
      return;
    }

    QString sException = value.property("name").toString();
    qint32 iLineNr = value.property("lineNumber").toInt() - 1;
    QString sStack = value.property("stack").toString();
    QString sError = "Uncaught " + sException +
                     " at line " + QString::number(iLineNr) +
                     ": " + value.toString() + "\n" + sStack;
    qCritical() << sError;

    emit spSignalEmitterContext->showError(sError, QtMsgType::QtCriticalMsg);
    emit spSignalEmitterContext->executionError(value.toString(), iLineNr, sStack);
  }

  //--------------------------------------------------------------------------------------
  //
  void Init()
  {
    m_pScriptEngine = new QJSEngine();
    m_pScriptEngine->installExtensions(QJSEngine::TranslationExtension |
                                       QJSEngine::ConsoleExtension |
                                       QJSEngine::GarbageCollectionExtension);

    m_pScriptUtils = new CScriptRunnerUtils(this, m_wpSignalEmiterContext.lock());
    connect(m_pScriptUtils, &CScriptRunnerUtils::finishedScript,
            this, &CJsScriptRunnerInstanceWorker::FinishedScript);

    // yes we need to call the static method of QQmlEngine, not QJSEngine, WHY Qt, WHY???
    QQmlEngine::setObjectOwnership(m_pScriptUtils, QQmlEngine::CppOwnership);
    QJSValue scriptValueUtils = m_pScriptEngine->newQObject(m_pScriptUtils);
    m_pScriptEngine->globalObject().setProperty("utils", scriptValueUtils);

    QJSValue enumIconObjectValue = m_pScriptEngine->newQMetaObject(&IconAlignment::staticMetaObject);
    m_pScriptEngine->globalObject().setProperty("IconAlignment", enumIconObjectValue);
    QJSValue enumTextObjectValue = m_pScriptEngine->newQMetaObject(&TextAlignment::staticMetaObject);
    m_pScriptEngine->globalObject().setProperty("TextAlignment", enumTextObjectValue);
  }

  //--------------------------------------------------------------------------------------
  //
  void Deinit()
  {
    InterruptExecution();

    ResetEngine();

    if (nullptr != m_pScriptEngine)
    {
      m_pScriptEngine->globalObject().setProperty("utils", QJSValue());
      m_pScriptEngine->collectGarbage();
    }

    delete m_pScriptUtils;
    m_pScriptUtils = nullptr;

    if (nullptr != m_pScriptEngine)
    {
      m_pScriptEngine->deleteLater();
    }
  }

  //--------------------------------------------------------------------------------------
  //
  void InterruptExecution()
  {
    if (nullptr != m_pScriptEngine)
    {
      auto spSignalEmitterContext = m_wpSignalEmiterContext.lock();
      if (nullptr != spSignalEmitterContext)
      {
        emit spSignalEmitterContext->interrupt();
      }
      m_pScriptEngine->setInterrupted(true);
    }
  }

  //----------------------------------------------------------------------------------------
  //
  void RunScript(const QString& sScript,
                 tspScene spScene, tspResource spResource)
  {
    // set scene
    if (nullptr != m_pCurrentScene)
    {
      delete m_pCurrentScene;
    }
    m_pCurrentScene = new CSceneScriptWrapper(m_pScriptEngine, spScene);
    // yes we need to call the static method of QQmlEngine, not QJSEngine, WHY Qt, WHY???
    QQmlEngine::setObjectOwnership(m_pCurrentScene, QQmlEngine::CppOwnership);
    QJSValue sceneValue = m_pScriptEngine->newQObject(m_pCurrentScene);
    m_pScriptEngine->globalObject().setProperty("scene", sceneValue);

    // set current Project
    for (auto it = m_objectMap.begin(); m_objectMap.end() != it; ++it)
    {
      it->second->SetCurrentProject(spResource->m_spParent);
    }

    m_pScriptUtils->SetCurrentProject(spResource->m_spParent);

    // resume engine if interrupetd
    m_pScriptEngine->setInterrupted(false);

    auto spSignalEmitterContext = m_wpSignalEmiterContext.lock();
    if (nullptr == spSignalEmitterContext)
    {
      QJSValue valDummy;
      HandleError(valDummy);
      return;
    }

    spSignalEmitterContext->SetScriptExecutionStatus(CScriptRunnerSignalEmiter::eRunning);

    // get scene name and set it as function, so error messages show the scene name in the error
    QReadLocker locker(&spScene->m_rwLock);
    m_spProject = spScene->m_spParent;
    const QString sSceneName = QString(spScene->m_sName).replace(QRegExp("\\W"), "_");
    locker.unlock();

    // create wrapper function to make syntax of scripts easier and handle return value
    // and be able to emit signal on finished
    QString sSkript = QString("(function() { "
                              "function include(resource) { return eval(utils.include(resource)); }; "
                              "var %1 = function() { %2\n}; "
                              "var ret = %3(); "
                              "utils.finishedScript(ret); \n "
                              "})")
        .arg(sSceneName)
        .arg(sScript)
        .arg(sSceneName);
    QJSValue runFunction = m_pScriptEngine->evaluate(sSkript);

    if (runFunction.isError())
    {
      spSignalEmitterContext->SetScriptExecutionStatus(CScriptRunnerSignalEmiter::eStopped);
      HandleError(runFunction);
      return;
    }

    if (runFunction.isCallable())
    {
      QJSValue ret = runFunction.call();
      spSignalEmitterContext->SetScriptExecutionStatus(CScriptRunnerSignalEmiter::eStopped);
      if (!m_pScriptEngine->isInterrupted())
      {
        if (ret.isError())
        {
          HandleError(ret);
        }
      }
      else
      {
        emit HandleScriptFinish(false, QString());
      }
    }
    else
    {
      spSignalEmitterContext->SetScriptExecutionStatus(CScriptRunnerSignalEmiter::eStopped);

      QString sError =  tr("Cannot call java-script.");
      qCritical() << sError;
      emit spSignalEmitterContext->showError(sError, QtMsgType::QtCriticalMsg);
      emit spSignalEmitterContext->executionError(sError, 0, "");

      emit HandleScriptFinish(false, QString());
    }
  }

  //--------------------------------------------------------------------------------------
  //
  void RegisterNewComponent(const QString sName, CScriptRunnerSignalEmiter* pObject)
  {
    auto it = m_objectMap.find(sName);
    if (m_objectMap.end() == it)
    {
      if (nullptr != pObject)
      {
        pObject->Initialize(m_wpSignalEmiterContext.lock());
        std::shared_ptr<CScriptObjectBase> spObject =
            pObject->CreateNewScriptObject(m_pScriptEngine);
        if (nullptr != spObject)
        {
          if (spObject->thread() != thread())
          {
            spObject->moveToThread(thread());
          }
          m_objectMap.insert({ sName, spObject });

          if (auto pNotification = dynamic_cast<CScriptNotification*>(spObject.get()))
          {
            connect(pNotification, &CScriptNotification::SignalOverlayCleared,
                    this, &CJsScriptRunnerInstanceWorker::SignalOverlayCleared);
            connect(pNotification, &CScriptNotification::SignalOverlayClosed,
                    this, &CJsScriptRunnerInstanceWorker::SignalOverlayClosed);
            connect(pNotification, &CScriptNotification::SignalOverlayRunAsync,
                    this, [this](const QString& sId, const QString& sScriptResource){
              emit SignalOverlayRunAsync(m_spProject, sId, sScriptResource);
            });
          }

          // yes we need to call the static method of QQmlEngine, not QJSEngine, WHY Qt, WHY???
          QQmlEngine::setObjectOwnership(spObject.get(), QQmlEngine::CppOwnership);
          QJSValue scriptValue = m_pScriptEngine->newQObject(spObject.get());
          m_pScriptEngine->globalObject().setProperty(sName, scriptValue);
        }
      }
    }
  }

  //--------------------------------------------------------------------------------------
  //
  void ResetEngine()
  {
    // remove ref to run function
    m_pScriptEngine->globalObject().setProperty("scene", QJSValue());

    m_pScriptUtils->SetCurrentProject(nullptr);

    for (auto it = m_objectMap.begin(); m_objectMap.end() != it; ++it)
    {
      m_pScriptEngine->globalObject().setProperty(it->first, QJSValue());
      it->second->Cleanup();
      it->second->SetCurrentProject(nullptr);
    }

    m_pScriptEngine->collectGarbage();

    if (nullptr != m_pCurrentScene)
    {
      delete m_pCurrentScene;
    }
  }

  //--------------------------------------------------------------------------------------
  //
  void UnregisterComponents()
  {
    ResetEngine();
    m_objectMap.clear();
  }

signals:
  void HandleScriptFinish(bool bSuccess, const QVariant& sRetVal);
  void SignalOverlayCleared();
  void SignalOverlayClosed(const QString& sId);
  void SignalOverlayRunAsync(tspProject spProject, const QString& sId, const QString& sScriptResource);

private:
  tspProject                                     m_spProject;
  std::weak_ptr<CScriptRunnerSignalContext>      m_wpSignalEmiterContext;
  QPointer<QJSEngine>                            m_pScriptEngine;
  QPointer<CScriptRunnerUtils>                   m_pScriptUtils;
  QPointer<CSceneScriptWrapper>                  m_pCurrentScene;
  std::map<QString /*name*/,
           std::shared_ptr<CScriptObjectBase>>   m_objectMap;
};

//----------------------------------------------------------------------------------------
//
class CJsScriptRunnerInstanceController : public QObject
{
  Q_OBJECT
public:
  CJsScriptRunnerInstanceController(const QString& sName,
                                    std::weak_ptr<CScriptRunnerSignalContext> wpSignalEmitterContext) :
    QObject(nullptr),
    m_spWorker(std::make_shared<CJsScriptRunnerInstanceWorker>(wpSignalEmitterContext)),
    m_pThread(new QThread(this))
  {
    qRegisterMetaType<CScriptRunnerSignalEmiter*>();

    connect(m_spWorker.get(), &CJsScriptRunnerInstanceWorker::HandleScriptFinish,
            this, &CJsScriptRunnerInstanceController::SlotHandleScriptFinish, Qt::QueuedConnection);

    connect(m_spWorker.get(), &CJsScriptRunnerInstanceWorker::SignalOverlayCleared,
            this, &CJsScriptRunnerInstanceController::SignalOverlayCleared, Qt::QueuedConnection);
    connect(m_spWorker.get(), &CJsScriptRunnerInstanceWorker::SignalOverlayClosed,
            this, &CJsScriptRunnerInstanceController::SignalOverlayClosed, Qt::QueuedConnection);
    connect(m_spWorker.get(), &CJsScriptRunnerInstanceWorker::SignalOverlayRunAsync,
            this, &CJsScriptRunnerInstanceController::SignalOverlayRunAsync, Qt::QueuedConnection);

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
  ~CJsScriptRunnerInstanceController()
  {
    bool bOk = QMetaObject::invokeMethod(m_spWorker.get(), "Deinit", Qt::BlockingQueuedConnection);
    assert(bOk); Q_UNUSED(bOk)
    m_pThread->quit();
    while (!m_pThread->isFinished())
    {
      m_pThread->wait(5);
    }
  }

  void InterruptExecution()
  {
    // direct call, or else we can't stop the script
    m_spWorker->InterruptExecution();
  }

  void RegisterNewComponent(const QString sName, CScriptRunnerSignalEmiter* pObject)
  {
    bool bOk = QMetaObject::invokeMethod(m_spWorker.get(), "RegisterNewComponent", Qt::BlockingQueuedConnection,
                                         Q_ARG(QString, sName),
                                         Q_ARG(CScriptRunnerSignalEmiter*, pObject));
    assert(bOk); Q_UNUSED(bOk)
  }

  void RunScript(const QString& sScript, tspScene spScene, tspResource spResource)
  {
    bool bOk = QMetaObject::invokeMethod(m_spWorker.get(), "RunScript", Qt::QueuedConnection,
                                         Q_ARG(QString, sScript),
                                         Q_ARG(tspScene, spScene),
                                         Q_ARG(tspResource, spResource));
    assert(bOk); Q_UNUSED(bOk)
  }

  void ResetEngine()
  {
    bool bOk = QMetaObject::invokeMethod(m_spWorker.get(), "ResetEngine", Qt::BlockingQueuedConnection);
    assert(bOk); Q_UNUSED(bOk)
  }

  void UnregisterComponents()
  {
    bool bOk = QMetaObject::invokeMethod(m_spWorker.get(), "UnregisterComponents", Qt::BlockingQueuedConnection);
    assert(bOk); Q_UNUSED(bOk)
  }

signals:
  void HandleScriptFinish(const QString& sName, bool bSuccess, const QVariant& sRetVal);
  void SignalOverlayCleared();
  void SignalOverlayClosed(const QString& sId);
  void SignalOverlayRunAsync(tspProject spProject, const QString& sId, const QString& sScriptResource);

private slots:
  void SlotHandleScriptFinish(bool bSuccess, const QVariant& sRetVal)
  {
    emit HandleScriptFinish(objectName(), bSuccess, sRetVal);
  }

private:
  std::shared_ptr<CJsScriptRunnerInstanceWorker> m_spWorker;
  QPointer<QThread>                              m_pThread;
};

//----------------------------------------------------------------------------------------
//
CJsScriptRunner::CJsScriptRunner(std::weak_ptr<CScriptRunnerSignalContext> spSignalEmitterContext,
                                 QObject* pParent) :
  QObject(pParent),
  IScriptRunner(),
  m_wpSignalEmitterContext(spSignalEmitterContext),
  m_runnerMutex(QMutex::Recursive),
  m_vspJsRunner(),
  m_signalEmiterMutex(QMutex::Recursive),
  m_pSignalEmiters()
{
}

CJsScriptRunner::~CJsScriptRunner()
{
  QMutexLocker locker(&m_runnerMutex);
  m_vspJsRunner.clear();
}

//----------------------------------------------------------------------------------------
//
void CJsScriptRunner::Initialize()
{
  QMutexLocker locker(&m_runnerMutex);
  m_vspJsRunner.insert({c_sMainRunner,
                        std::make_shared<CJsScriptRunnerInstanceController>(
                          c_sMainRunner, m_wpSignalEmitterContext)});
  auto it = m_vspJsRunner.find(c_sMainRunner);
  it->second->setObjectName(c_sMainRunner);
  connect(it->second.get(), &CJsScriptRunnerInstanceController::HandleScriptFinish,
          this, &CJsScriptRunner::SlotHandleScriptFinish);

  connect(it->second.get(), &CJsScriptRunnerInstanceController::SignalOverlayCleared,
          this, &CJsScriptRunner::SlotOverlayCleared);
  connect(it->second.get(), &CJsScriptRunnerInstanceController::SignalOverlayClosed,
          this, &CJsScriptRunner::SlotOverlayClosed);
  connect(it->second.get(), &CJsScriptRunnerInstanceController::SignalOverlayRunAsync,
          this, &CJsScriptRunner::SlotOverlayRunAsync);
}

//----------------------------------------------------------------------------------------
//
void CJsScriptRunner::Deinitialize()
{
  QMutexLocker locker(&m_runnerMutex);
  m_vspJsRunner.clear();
}

//----------------------------------------------------------------------------------------
//
void CJsScriptRunner::InterruptExecution()
{
  QMutexLocker locker(&m_runnerMutex);
  for (auto& it : m_vspJsRunner)
  {
    it.second->InterruptExecution();
  }
}

//----------------------------------------------------------------------------------------
//
void CJsScriptRunner::PauseExecution()
{
}

//----------------------------------------------------------------------------------------
//
void CJsScriptRunner::ResumeExecution()
{
}

//----------------------------------------------------------------------------------------
//
void CJsScriptRunner::LoadScript(const QString& sScript,
                                 tspScene spScene, tspResource spResource)
{
  RunScript(c_sMainRunner, sScript, spScene, spResource);
}

//----------------------------------------------------------------------------------------
//
void CJsScriptRunner::RegisterNewComponent(const QString sName, QJSValue signalEmitter)
{
  QMutexLocker locker(&m_runnerMutex);
  for (auto& it : m_vspJsRunner)
  {
    if (signalEmitter.isObject())
    {
      CScriptRunnerSignalEmiter* pObject =
          qobject_cast<CScriptRunnerSignalEmiter*>(signalEmitter.toQObject());
      it.second->RegisterNewComponent(sName, pObject);

      QMutexLocker lockerEmiter(&m_signalEmiterMutex);
      m_pSignalEmiters.insert({sName, pObject});
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CJsScriptRunner::UnregisterComponents()
{
  QMutexLocker locker(&m_runnerMutex);
  for (auto& it : m_vspJsRunner)
  {
    it.second->UnregisterComponents();
  }

  QMutexLocker lockerEmiter(&m_signalEmiterMutex);
  m_pSignalEmiters.clear();
}

//----------------------------------------------------------------------------------------
//
void CJsScriptRunner::SlotHandleScriptFinish(const QString& sName, bool bSuccess, const QVariant& sRetVal)
{
  auto spSignalEmitterContext = m_wpSignalEmitterContext.lock();
  if (nullptr == spSignalEmitterContext)
  {
    qWarning() << "m_wpSignalEmitterContext was null";
    return;
  }

  {
    QMutexLocker locker(&m_runnerMutex);
    auto it = m_vspJsRunner.find(sName);
    if (m_vspJsRunner.end() == it)
    {
      qWarning() << QString("JS Script runner %1 not found").arg(sName);
      return;
    }

    it->second->ResetEngine();

    if (c_sMainRunner != sName)
    {
      m_vspJsRunner.erase(it);
    }
  }

  if (!bSuccess)
  {
    qWarning() << tr("Error in script, unloading project.");
  }

  if (c_sMainRunner == sName || QVariant::String == sRetVal.type())
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
void CJsScriptRunner::SlotOverlayCleared()
{
  QMutexLocker locker(&m_runnerMutex);
  auto it = m_vspJsRunner.begin();
  while (m_vspJsRunner.end() != it)
  {
    if (c_sMainRunner != it->first)
    {
      it->second->ResetEngine();
      it = m_vspJsRunner.erase(it);
    }
    else
    {
      ++it;
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CJsScriptRunner::SlotOverlayClosed(const QString& sId)
{
  QMutexLocker locker(&m_runnerMutex);
  auto it = m_vspJsRunner.find(sId);
  if (m_vspJsRunner.end() == it)
  {
    qWarning() << QString("JS Script runner %1 not found").arg(sId);
    return;
  }

  it->second->ResetEngine();

  if (c_sMainRunner != sId)
  {
    m_vspJsRunner.erase(it);
  }
}

//----------------------------------------------------------------------------------------
//
void CJsScriptRunner::SlotOverlayRunAsync(tspProject spProject, const QString& sId, const QString& sScriptResource)
{
  {
    QMutexLocker locker(&m_runnerMutex);
    m_vspJsRunner.insert({sId,
                          std::make_shared<CJsScriptRunnerInstanceController>(
                            sId, m_wpSignalEmitterContext)});
    auto it = m_vspJsRunner.find(sId);
    it->second->setObjectName(sId);
    connect(it->second.get(), &CJsScriptRunnerInstanceController::HandleScriptFinish,
            this, &CJsScriptRunner::SlotHandleScriptFinish);

    connect(it->second.get(), &CJsScriptRunnerInstanceController::SignalOverlayCleared,
            this, &CJsScriptRunner::SlotOverlayCleared);
    connect(it->second.get(), &CJsScriptRunnerInstanceController::SignalOverlayClosed,
            this, &CJsScriptRunner::SlotOverlayClosed);
    connect(it->second.get(), &CJsScriptRunnerInstanceController::SignalOverlayRunAsync,
            this, &CJsScriptRunner::SlotOverlayRunAsync);

    QMutexLocker lockerEmiter(&m_signalEmiterMutex);
    for (auto& itEmiter : m_pSignalEmiters)
    {
      it->second->RegisterNewComponent(itEmiter.first, itEmiter.second);
    }
  }

  auto spSignalEmitterContext = m_wpSignalEmitterContext.lock();
  if (nullptr == spSignalEmitterContext)
  {
    qWarning() << "m_wpSignalEmitterContext was null";
    return;
  }

  if (auto spDbManager = CApplication::Instance()->System<CDatabaseManager>().lock())
  {
    tspResource spResource = spDbManager->FindResourceInProject(spProject, sScriptResource);

    if (nullptr == spResource ||
        nullptr == spResource->m_spParent)
    {
      QString sError = tr("Script file, Scene or Project is null");
      qCritical() << sError;
      emit spSignalEmitterContext->showError(sError, QtMsgType::QtCriticalMsg);
      return;
    }

    QReadLocker projectLocker(&spResource->m_spParent->m_rwLock);
    QReadLocker resourceLocker(&spResource->m_rwLock);
    if (spResource->m_type._to_integral() != EResourceType::eScript)
    {
      QString sError = tr("Script resource is of wrong type.");
      qCritical() << sError;
      emit spSignalEmitterContext->showError(sError, QtMsgType::QtCriticalMsg);
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
        RunScript(sId, sScript, nullptr, spResource);
      }
      else
      {
        QString sError = tr("Script resource file could not be opened.");
        qCritical() << sError;
        emit spSignalEmitterContext->showError(sError, QtMsgType::QtCriticalMsg);
        return;
      }
    }
    else
    {
      QString sError = tr("Script resource file does not exist.");
      qCritical() << sError;
      emit spSignalEmitterContext->showError(sError, QtMsgType::QtCriticalMsg);
      return;
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CJsScriptRunner::RunScript(const QString& sId, const QString& sScript,
                                tspScene spScene, tspResource spResource)
{
  QMutexLocker locker(&m_runnerMutex);
  auto it = m_vspJsRunner.find(sId);
  if (m_vspJsRunner.end() != it)
  {
    it->second->InterruptExecution();
    it->second->RunScript(sScript, spScene, spResource);
  }
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CScriptRunnerSignalContext> CJsScriptRunner::SignalEmmitterContext()
{
  return m_wpSignalEmitterContext.lock();
}

//----------------------------------------------------------------------------------------
//
CScriptRunnerUtils::CScriptRunnerUtils(QObject* pParent,
                                       std::shared_ptr<CScriptRunnerSignalContext> spSignalEmiterContext) :
  QObject(pParent),
  m_spSignalEmiterContext(spSignalEmiterContext)
{

}
CScriptRunnerUtils::~CScriptRunnerUtils()
{

}

//----------------------------------------------------------------------------------------
//
void CScriptRunnerUtils::SetCurrentProject(tspProject spProject)
{
  m_spProject = spProject;
}

//----------------------------------------------------------------------------------------
//
QString CScriptRunnerUtils::include(QJSValue resource)
{
  auto spDbManager = CApplication::Instance()->System<CDatabaseManager>().lock();
  tspResource spResource;
  if (nullptr != spDbManager)
  {
    if (resource.isString())
    {
      QString sResource = resource.toString();
      spResource = spDbManager->FindResourceInProject(m_spProject, sResource);
      if (nullptr == spResource)
      {
        QString sError = tr("Resource %1 not found");
        emit m_spSignalEmiterContext
            ->showError(sError.arg(resource.toString()),QtMsgType::QtWarningMsg);
      }
    }
    else if (resource.isQObject())
    {
      CResourceScriptWrapper* pResource = dynamic_cast<CResourceScriptWrapper*>(resource.toQObject());
      if (nullptr != pResource)
      {
        if (nullptr != pResource->Data())
        {
          spResource = pResource->Data();
        }
        else
        {
          QString sError = tr("Resource in include() holds no data.");
          emit m_spSignalEmiterContext
              ->showError(sError.arg(resource.toString()),QtMsgType::QtWarningMsg);
        }
      }
      else
      {
        QString sError = tr("Wrong argument-type to include(). String or resource was expected.");
        emit m_spSignalEmiterContext
            ->showError(sError.arg(resource.toString()),QtMsgType::QtWarningMsg);
      }
    }
    else
    {
      QString sError = tr("Wrong argument-type to include(). String or resource was expected.");
      emit m_spSignalEmiterContext
          ->showError(sError.arg(resource.toString()),QtMsgType::QtWarningMsg);
    }
  }


  if (nullptr != spResource)
  {
    QReadLocker locker(&spResource->m_rwLock);
    if (EResourceType::eScript == spResource->m_type._to_integral())
    {
      QString sPath = ResourceUrlToAbsolutePath(spResource);
      QFile sciptFile(sPath);
      if (sciptFile.exists() && sciptFile.open(QIODevice::ReadOnly))
      {
        return "(" + QString::fromUtf8(sciptFile.readAll()) + ")";
      }
    }
  }
  return QString("({})");
}

#include "JsScriptRunner.moc"
