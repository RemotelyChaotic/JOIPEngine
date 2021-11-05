#include "JsScriptRunner.h"
#include "Application.h"
#include "ScriptRunnerSignalEmiter.h"
#include "ScriptTextBox.h"
#include "Systems/Project.h"
#include "Systems/Resource.h"
#include "Systems/Scene.h"

#include <QDebug>
#include <QFile>
#include <QQmlEngine>
#include <QTimer>

CJsScriptRunner::CJsScriptRunner(std::weak_ptr<CScriptRunnerSignalContext> spSignalEmitterContext,
                                 QObject* pParent) :
  QObject(pParent),
  IScriptRunner(),
  m_spTimer(std::make_shared<QTimer>()),
  m_wpSignalEmitterContext(spSignalEmitterContext),
  m_pScriptEngine(new QJSEngine()),
  m_pScriptUtils(new CScriptRunnerUtils(this, this)),
  m_pCurrentScene(nullptr),
  m_runFunction(),
  m_objectMapMutex(QMutex::Recursive),
  m_objectMap()
{
  connect(m_spTimer.get(), &QTimer::timeout, this, &CJsScriptRunner::SlotRun);
}

CJsScriptRunner::~CJsScriptRunner()
{
  if (nullptr != m_pScriptEngine)
  {
    m_pScriptEngine->deleteLater();
  }
}

//----------------------------------------------------------------------------------------
//
void CJsScriptRunner::Initialize()
{
  m_pScriptEngine->installExtensions(QJSEngine::TranslationExtension |
                                     QJSEngine::ConsoleExtension |
                                     QJSEngine::GarbageCollectionExtension);

  connect(m_pScriptUtils, &CScriptRunnerUtils::finishedScript,
          this, &CJsScriptRunner::SlotFinishedScript);

  // yes we need to call the static method of QQmlEngine, not QJSEngine, WHY Qt, WHY???
  QQmlEngine::setObjectOwnership(m_pScriptUtils, QQmlEngine::CppOwnership);
  QJSValue scriptValueUtils = m_pScriptEngine->newQObject(m_pScriptUtils);
  m_pScriptEngine->globalObject().setProperty("utils", scriptValueUtils);

  QJSValue enumTextObjectValue = m_pScriptEngine->newQMetaObject(&TextAlignment::staticMetaObject);
  m_pScriptEngine->globalObject().setProperty("TextAlignment", enumTextObjectValue);
}

//----------------------------------------------------------------------------------------
//
void CJsScriptRunner::Deinitialize()
{
  m_spTimer->stop();

  m_spScriptEngine->globalObject().setProperty("utils", QJSValue());
  delete m_pScriptUtils;
  m_pScriptUtils = nullptr;

  m_spTimer->stop();
  m_spTimer = nullptr;

  m_pScriptEngine->deleteLater();
}

//----------------------------------------------------------------------------------------
//
void CJsScriptRunner::InterruptExecution()
{
  m_pScriptEngine->setInterrupted(true);
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
  m_objectMapMutex.lock();
  for (auto it = m_objectMap.begin(); m_objectMap.end() != it; ++it)
  {
    it->second->SetCurrentProject(spResource->m_spParent);
  }
  m_objectMapMutex.unlock();

  m_pScriptUtils->SetCurrentProject(spResource->m_spParent);

  // resume engine if interrupetd
  m_pScriptEngine->setInterrupted(false);

  auto spSignalEmitterContext = m_wpSignalEmitterContext.lock();
  if (nullptr == spSignalEmitterContext)
  {
    HandleError(m_runFunction);
    return;
  }

  spSignalEmitterContext->SetScriptExecutionStatus(CScriptRunnerSignalEmiter::eRunning);

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
  m_runFunction = m_pScriptEngine->evaluate(sSkript);

  if (m_runFunction.isError())
  {
    spSignalEmitterContext->SetScriptExecutionStatus(CScriptRunnerSignalEmiter::eStopped);
    HandleError(m_runFunction);
    return;
  }

  m_spTimer->setSingleShot(true);
  m_spTimer->start(10);
}

//----------------------------------------------------------------------------------------
//
void CJsScriptRunner::RegisterNewComponent(const QString sName, QJSValue signalEmitter)
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
          pObject->CreateNewScriptObject(m_pScriptEngine);
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
void CJsScriptRunner::UnregisterComponents()
{
  // remove ref to run function
  m_runFunction = QJSValue();

  m_pScriptEngine->globalObject().setProperty("scene", QJSValue());

  m_objectMapMutex.lock();
  for (auto it = m_objectMap.begin(); m_objectMap.end() != it; ++it)
  {
    m_pScriptEngine->globalObject().setProperty(it->first, QJSValue());
    it->second->Cleanup();
  }
  m_objectMapMutex.unlock();

  m_pScriptEngine->collectGarbage();

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
void CJsScriptRunner::HandleScriptFinish(bool bSuccess, const QVariant& sRetVal)
{
  auto spSignalEmitterContext = m_wpSignalEmitterContext.lock();
  if (nullptr == spSignalEmitterContext)
  {
    HandleError(m_runFunction);
    return;
  }

  spSignalEmitterContext->SetScriptExecutionStatus(CScriptRunnerSignalEmiter::eStopped);

  m_pScriptEngine->globalObject().setProperty("scene", QJSValue());

  m_pScriptEngine->collectGarbage();

  if (nullptr != m_pCurrentScene)
  {
    delete m_pCurrentScene;
  }

  emit spSignalEmitterContext->interrupt();

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
std::shared_ptr<CScriptRunnerSignalContext> CJsScriptRunner::SignalEmmitterContext()
{
  return m_wpSignalEmitterContext.lock();
}

//----------------------------------------------------------------------------------------
//
void CJsScriptRunner::SlotFinishedScript(const QVariant& sRetVal)
{
  HandleScriptFinish(true, sRetVal);
}

//----------------------------------------------------------------------------------------
//
void CJsScriptRunner::SlotRegisterObject(const QString& sObject)
{
  QMutexLocker locker(&m_objectMapMutex);
  auto it = m_objectMap.find(sObject);
  if (m_objectMap.end() != it)
  {
    // yes we need to call the static method of QQmlEngine, not QJSEngine, WHY Qt, WHY???
    QQmlEngine::setObjectOwnership(it->second.get(), QQmlEngine::CppOwnership);
    QJSValue scriptValue = m_pScriptEngine->newQObject(it->second.get());
    m_pScriptEngine->globalObject().setProperty(it->first, scriptValue);
  }
}

//----------------------------------------------------------------------------------------
//
void CJsScriptRunner::SlotRun()
{
  auto spSignalEmitterContext = m_wpSignalEmitterContext.lock();
  if (nullptr == spSignalEmitterContext)
  {
    HandleError(m_runFunction);
    return;
  }

  if (m_runFunction.isCallable())
  {
    QJSValue ret = m_runFunction.call();
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
      HandleScriptFinish(false, QString());
    }
  }
  else
  {
    spSignalEmitterContext->SetScriptExecutionStatus(CScriptRunnerSignalEmiter::eStopped);

    QString sError =  tr("Cannot call java-script.");
    qCritical() << sError;
    emit spSignalEmitterContext->showError(sError, QtMsgType::QtCriticalMsg);
    emit spSignalEmitterContext->executionError(sError, 0, "");

    HandleScriptFinish(false, QString());
  }
}

//----------------------------------------------------------------------------------------
//
void CJsScriptRunner::HandleError(QJSValue& value)
{
  QString sException = value.property("name").toString();
  qint32 iLineNr = value.property("lineNumber").toInt() - 1;
  QString sStack = value.property("stack").toString();
  QString sError = "Uncaught " + sException +
                   " at line " + QString::number(iLineNr) +
                   ": " + value.toString() + "\n" + sStack;
  qCritical() << sError;

  auto spSignalEmitterContext = m_wpSignalEmitterContext.lock();
  if (nullptr == spSignalEmitterContext)
  {
    qCritical() << "SignalEmitter is null";
    return;
  }

  emit spSignalEmitterContext->showError(sError, QtMsgType::QtCriticalMsg);
  emit spSignalEmitterContext->executionError(value.toString(), iLineNr, sStack);
}


//----------------------------------------------------------------------------------------
//
CScriptRunnerUtils::CScriptRunnerUtils(QObject* pParent,
                                       QPointer<CJsScriptRunner> pScriptRunner) :
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
