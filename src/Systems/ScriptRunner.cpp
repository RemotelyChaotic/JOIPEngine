#include "ScriptRunner.h"
#include "Application.h"
#include "Project.h"
#include "Resource.h"
#include "Scene.h"
#include "Script/ScriptBackground.h"
#include "Script/ScriptIcon.h"
#include "Script/ScriptMediaPlayer.h"
#include "Script/ScriptRunnerSignalEmiter.h"
#include "Script/ScriptStorage.h"
#include "Script/ScriptTextBox.h"
#include "Script/ScriptTimer.h"
#include "Script/ScriptThread.h"
#include "Settings.h"

#include <QDebug>
#include <QFileInfo>
#include <QQmlEngine>

//----------------------------------------------------------------------------------------
//
CScriptRunner::CScriptRunner() :
  CSystemBase(),
  m_spSettings(CApplication::Instance()->Settings()),
  m_spScriptEngine(nullptr),
  m_spSignalEmitterContext(nullptr),
  m_spTimer(nullptr),
  m_objectMapMutex(),
  m_objectMap(),
  m_pScriptUtils(nullptr),
  m_pCurrentScene(nullptr),
  m_runFunction()
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
  m_spScriptEngine.reset(new QJSEngine());

  m_spScriptEngine->installExtensions(QJSEngine::TranslationExtension |
                                      QJSEngine::ConsoleExtension |
                                      QJSEngine::GarbageCollectionExtension);

  m_spSignalEmitterContext = std::make_shared<CScriptRunnerSignalContext>();
  m_spTimer = std::make_shared<QTimer>();
  connect(m_spTimer.get(), &QTimer::timeout, this, &CScriptRunner::SlotRun);

  m_pScriptUtils = new CScriptRunnerUtils(this, this);
  connect(m_pScriptUtils, &CScriptRunnerUtils::finishedScript,
          this, &CScriptRunner::SlotFinishedScript);

  // yes we need to call the static method of QQmlEngine, not QJSEngine, WHY Qt, WHY???
  QQmlEngine::setObjectOwnership(m_pScriptUtils, QQmlEngine::CppOwnership);
  QJSValue scriptValueUtils = m_spScriptEngine->newQObject(m_pScriptUtils);
  m_spScriptEngine->globalObject().setProperty("utils", scriptValueUtils);

  QJSValue enumTextObjectValue = m_spScriptEngine->newQMetaObject(&TextAlignment::staticMetaObject);
  m_spScriptEngine->globalObject().setProperty("TextAlignment", enumTextObjectValue);

  SetInitialized(true);
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::Deinitialize()
{
  InterruptExecution();

  m_spScriptEngine->globalObject().setProperty("utils", QJSValue());
  delete m_pScriptUtils;
  m_pScriptUtils = nullptr;

  m_spTimer->stop();
  m_spTimer = nullptr;

  UnregisterComponents();

  m_spSignalEmitterContext = nullptr;
  m_spScriptEngine.reset();

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

  QReadLocker projectLocker(&spResource->m_spParent->m_rwLock);
  QReadLocker resourceLocker(&spResource->m_rwLock);
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

      // set scene
      if (nullptr != m_pCurrentScene)
      {
        delete m_pCurrentScene;
      }
      m_pCurrentScene = new CScene(m_spScriptEngine.get(), spScene);
      // yes we need to call the static method of QQmlEngine, not QJSEngine, WHY Qt, WHY???
      QQmlEngine::setObjectOwnership(m_pCurrentScene, QQmlEngine::CppOwnership);
      QJSValue sceneValue = m_spScriptEngine->newQObject(m_pCurrentScene);
      m_spScriptEngine->globalObject().setProperty("scene", sceneValue);

      // set current Project
      m_objectMapMutex.lock();
      for (auto it = m_objectMap.begin(); m_objectMap.end() != it; ++it)
      {
        it->second->SetCurrentProject(spResource->m_spParent);
      }
      m_objectMapMutex.unlock();

      m_pScriptUtils->SetCurrentProject(spResource->m_spParent);

      // resume engine if interrupetd
      m_spScriptEngine->setInterrupted(false);
      m_spSignalEmitterContext->SetScriptExecutionStatus(CScriptRunnerSignalEmiter::eRunning);

      // get scene name and set it as function, so error messages show the scene name in the error
      QReadLocker locker(&spScene->m_rwLock);
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
      m_runFunction = m_spScriptEngine->evaluate(sSkript);

      if (m_runFunction.isError())
      {
        m_spSignalEmitterContext->SetScriptExecutionStatus(CScriptRunnerSignalEmiter::eStopped);
        HandleError(m_runFunction);
        return;
      }

      m_spTimer->setSingleShot(true);
      m_spTimer->start(10);
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
  m_spScriptEngine->setInterrupted(true);
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
  QMutexLocker locker(&m_objectMapMutex);
  auto it = m_objectMap.find(sName);
  if (m_objectMap.end() == it)
  {
    if (signalEmitter.isObject())
    {
      CScriptRunnerSignalEmiter* pObject =
          qobject_cast<CScriptRunnerSignalEmiter*>(signalEmitter.toQObject());
      pObject->Initialize(m_spSignalEmitterContext);
      std::shared_ptr<CScriptObjectBase> spObject =
          pObject->CreateNewScriptObject(m_spScriptEngine.get());
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
void CScriptRunner::UnregisterComponents()
{
  m_spScriptEngine->globalObject().setProperty("scene", QJSValue());

  m_objectMapMutex.lock();
  for (auto it = m_objectMap.begin(); m_objectMap.end() != it; ++it)
  {
    m_spScriptEngine->globalObject().setProperty(it->first, QJSValue());
    it->second->Cleanup();
  }
  m_objectMapMutex.unlock();

  m_spScriptEngine->collectGarbage();

  if (nullptr != m_pCurrentScene)
  {
    delete m_pCurrentScene;
  }

  m_objectMapMutex.lock();
  m_objectMap.clear();
  m_objectMapMutex.unlock();
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::SlotFinishedScript(const QVariant& sRetVal)
{
  HandleScriptFinish(true, sRetVal);
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::SlotRun()
{
  if (m_runFunction.isCallable())
  {
    QJSValue ret = m_runFunction.call();
    m_spSignalEmitterContext->SetScriptExecutionStatus(CScriptRunnerSignalEmiter::eStopped);
    if (!m_spScriptEngine->isInterrupted())
    {
      if (ret.isError())
      {
        HandleError(ret);
      }
    }
    else
    {
      HandleScriptFinish(false, QString());
    }
  }
  else
  {
    m_spSignalEmitterContext->SetScriptExecutionStatus(CScriptRunnerSignalEmiter::eStopped);

    QString sError =  tr("Cannot call java-script.");
    qCritical() << sError;
    emit m_spSignalEmitterContext->showError(sError, QtMsgType::QtCriticalMsg);
    emit m_spSignalEmitterContext->executionError(sError, 0, "");

    HandleScriptFinish(false, QString());
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::SlotRegisterObject(const QString& sObject)
{
  QMutexLocker locker(&m_objectMapMutex);
  auto it = m_objectMap.find(sObject);
  if (m_objectMap.end() != it)
  {
    // yes we need to call the static method of QQmlEngine, not QJSEngine, WHY Qt, WHY???
    QQmlEngine::setObjectOwnership(it->second.get(), QQmlEngine::CppOwnership);
    QJSValue scriptValue = m_spScriptEngine->newQObject(it->second.get());
    m_spScriptEngine->globalObject().setProperty(it->first, scriptValue);
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::HandleError(QJSValue& value)
{
  QString sException = value.property("name").toString();
  qint32 iLineNr = value.property("lineNumber").toInt() - 1;
  QString sStack = value.property("stack").toString();
  QString sError = "Uncaught " + sException +
                   " at line " + QString::number(iLineNr) +
                   ": " + value.toString() + "\n" + sStack;
  qCritical() << sError;
  emit m_spSignalEmitterContext->showError(sError, QtMsgType::QtCriticalMsg);
  emit m_spSignalEmitterContext->executionError(value.toString(), iLineNr, sStack);
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::HandleScriptFinish(bool bSuccess, const QVariant& sRetVal)
{
  m_spSignalEmitterContext->SetScriptExecutionStatus(CScriptRunnerSignalEmiter::eStopped);

  m_spScriptEngine->globalObject().setProperty("scene", QJSValue());

  m_spScriptEngine->collectGarbage();

  if (nullptr != m_pCurrentScene)
  {
    delete m_pCurrentScene;
  }

  emit m_spSignalEmitterContext->interrupt();

  m_pScriptUtils->SetCurrentProject(nullptr);

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
CScriptRunnerUtils::CScriptRunnerUtils(QObject* pParent, QPointer<CScriptRunner> pScriptRunner) :
  QObject(pParent),
  m_pScriptRunner(pScriptRunner)
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
        emit m_pScriptRunner->SignalEmmitterContext()
            ->showError(sError.arg(resource.toString()),QtMsgType::QtWarningMsg);
      }
    }
    else if (resource.isQObject())
    {
      CResource* pResource = dynamic_cast<CResource*>(resource.toQObject());
      if (nullptr != pResource)
      {
        if (nullptr != pResource->Data())
        {
          spResource = pResource->Data();
        }
        else
        {
          QString sError = tr("Resource in include() holds no data.");
          emit m_pScriptRunner->SignalEmmitterContext()
              ->showError(sError.arg(resource.toString()),QtMsgType::QtWarningMsg);
        }
      }
      else
      {
        QString sError = tr("Wrong argument-type to include(). String or resource was expected.");
        emit m_pScriptRunner->SignalEmmitterContext()
            ->showError(sError.arg(resource.toString()),QtMsgType::QtWarningMsg);
      }
    }
    else
    {
      QString sError = tr("Wrong argument-type to include(). String or resource was expected.");
      emit m_pScriptRunner->SignalEmmitterContext()
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
