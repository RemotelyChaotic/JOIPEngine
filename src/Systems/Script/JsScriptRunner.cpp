#include "JsScriptRunner.h"
#include "Application.h"
#include "ScriptCacheFileEngine.h"
#include "ScriptDbWrappers.h"
#include "ScriptNotification.h"
#include "ScriptRunnerInstanceController.h"
#include "ScriptRunnerSignalEmiter.h"
#include "ScriptTextBox.h"
#include "ScriptThread.h"

#include "Systems/PhysFs/PhysFsFileEngine.h"

#include "Systems/Project.h"
#include "Systems/Resource.h"
#include "Systems/Scene.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QQmlEngine>
#include <QTimer>

#include <functional>
#include <vector>

enum EImportMode
{
  eModuleName,
  eVariableName
};
struct SReplaceExpModule
{
  QString m_sFrom;
  QString m_sTo;
};
struct SReplaceExp
{
  QString m_sModule;
  std::vector<SReplaceExpModule> m_vsExps;
  qint32 m_iBegin;
  qint32 m_iNum;
  EImportMode m_mode;
};

namespace  {
  /*
   * import "module-name";
   * import name from "module-name";
   * import * as name from "module-name";
   * import { member } from "module-name";
   * import { member as alias } from "module-name";
   * import { member1 , member2 as alias2 , [...] } from "module-name";
  */
  const char* c_rxImportStatement0 = R"(import\s+\"(.*)\")";
  const char* c_rxImportStatement1 = R"(import\s+([a-zA-Z0-9_]+)\s+from\s+\"(.*)\")";
  const char* c_rxImportStatement2 = R"(import\s+\*\s+as\s+([a-zA-Z0-9_]+)\s+from\s+\"(.*)\")";
  const char* c_rxImportStatement3 = R"(import\s+\{\s*(.+)\s*\}\s*from\s+\"(.*)\")";
  const QString c_rxImportStatementFinal = QString("(%1)||(%2)||(%3)||(%4)")
      .arg(c_rxImportStatement0).arg(c_rxImportStatement1)
      .arg(c_rxImportStatement2).arg(c_rxImportStatement3);

  using tfnImportConstructor = std::function<QString(const SReplaceExp&)>;

  QString ReplaceImportStatements(QString& sScript, tfnImportConstructor fnConst)
  {
    QRegularExpression rx(c_rxImportStatementFinal,
                          QRegularExpression::MultilineOption);

    // collect strings to replace
    auto i = rx.globalMatch(sScript);
    QString sEntireMatch;
    QString sModule;
    QString sExp;
    std::vector<SReplaceExp> vExps;
    while (i.hasNext())
    {
      QRegularExpressionMatch match = i.next();
      //qDebug() << match.capturedTexts();
      //sEntireMatch = match.captured(0); // DEBUG
      sModule = match.captured(match.lastCapturedIndex());
      sExp = match.captured(match.lastCapturedIndex()-1);
      if (!sModule.isEmpty())
      {
        EImportMode mode = eVariableName;
        if (match.capturedStart(1) != -1 ||
            match.capturedStart(6) != -1)
        {
          mode = eModuleName;
        }
        qint32 iBegin = match.capturedStart();
        qint32 iNum = match.capturedEnd() - iBegin;
        if (sExp.contains("import")) { sExp = QString(); }

        std::vector<SReplaceExpModule> sExps;
        if (!sExp.isEmpty())
        {
          QStringList vsExpsModule = sExp.split(",");
          for (const QString& sExpModule : qAsConst(vsExpsModule))
          {
            QStringList vsExpModulkeParts = sExpModule.split(" as ");
            if (vsExpModulkeParts.size() > 1)
            {
              sExps.push_back({vsExpModulkeParts[0].trimmed(), vsExpModulkeParts[1].trimmed()});
            }
            else
            {
              sExps.push_back({vsExpModulkeParts[0].trimmed(), vsExpModulkeParts[0].trimmed()});
            }
          }
        }
        vExps.push_back({sModule, sExps, iBegin, iNum, mode});
      }
    }

    // replace them from the back
    for (qint32 i = static_cast<qint32>(vExps.size())-1; 0 <= i; --i)
    {
      SReplaceExp& exp = vExps[i];
      QString sNewExp = fnConst(exp);
      sScript.replace(exp.m_iBegin, exp.m_iNum, sNewExp);
    }

    return sScript;
  }

  //--------------------------------------------------------------------------------------
  //
  QString EvalImportReplacer(const SReplaceExp& exp)
  {
    QString sNewExp = QString("utils_1337.import(\"%1\");").arg(exp.m_sModule);
    if (exp.m_vsExps.size() > 0)
    {
      if (eModuleName == exp.m_mode)
      {
        sNewExp.prepend(QString("var %1 = ").arg(exp.m_vsExps.front().m_sTo));
      }
      else
      {
        sNewExp.prepend(QString("var _var_temp_dont_asign = "));
        for (auto& exp : exp.m_vsExps)
        {
          sNewExp.append(QString("var %2 = _var_temp_dont_asign.%1;").arg(exp.m_sFrom).arg(exp.m_sTo));
        }
      }
    }
    return sNewExp;
  }
}

//----------------------------------------------------------------------------------------
//
class CJsScriptRunnerInstanceWorker : public CScriptRunnerInstanceWorkerBase
{
  Q_OBJECT
public:
  CJsScriptRunnerInstanceWorker(const QString& sName,
                                std::weak_ptr<CScriptRunnerSignalContext> wpSignalEmitterContext) :
    CScriptRunnerInstanceWorkerBase(sName, wpSignalEmitterContext),
    m_pScriptEngine(nullptr),
    m_pScriptUtils(nullptr),
    m_pCurrentScene(nullptr),
    m_pCurrentProject(nullptr)
  {}
  ~CJsScriptRunnerInstanceWorker()
  {
  }

public slots:
  //----------------------------------------------------------------------------------------
  //
  void HandleError(QJSValue& value)
  {
    auto spSignalEmitterContext = m_wpSignalEmiterContext.lock();
    if (nullptr == spSignalEmitterContext)
    {
      qWarning() << "SignalEmitter is null";
      m_bRunning = 0;
      return;
    }

    QString sException = value.property("name").toString();
    qint32 iLineNr = value.property("lineNumber").toInt() - 1;
    QString sStack = value.property("stack").toString();
    QString sError = "Uncaught " + sException +
                     " at line " + QString::number(iLineNr) +
                     ": " + value.toString() + "\n" + sStack;
    qWarning() << sError;

    emit spSignalEmitterContext->showError(sError, QtMsgType::QtCriticalMsg);
    emit spSignalEmitterContext->executionError(value.toString(), iLineNr, sStack);

    m_bRunning = 0;
  }

  //--------------------------------------------------------------------------------------
  //
  void Init() override
  {
    m_pScriptEngine = new QQmlEngine();
    m_pScriptEngine->installExtensions(QJSEngine::TranslationExtension |
                                       QJSEngine::ConsoleExtension |
                                       QJSEngine::GarbageCollectionExtension);

    m_pUrlInterceptor = new CResourceUrlInterceptor();
    m_pScriptEngine->setUrlInterceptor(m_pUrlInterceptor);

    m_pScriptUtils = new CScriptRunnerUtilsJs(this, m_pScriptEngine, m_wpSignalEmiterContext.lock());
    connect(m_pScriptUtils, &CScriptRunnerUtilsJs::finishedScript,
            this, &CJsScriptRunnerInstanceWorker::FinishedScript);

    // yes we need to call the static method of QQmlEngine, not QJSEngine, WHY Qt, WHY???
    QQmlEngine::setObjectOwnership(m_pScriptUtils, QQmlEngine::CppOwnership);
    QJSValue scriptValueUtils = m_pScriptEngine->newQObject(m_pScriptUtils);
    m_pScriptEngine->globalObject().setProperty("utils_1337", scriptValueUtils);

    QJSValue enumIconObjectValue = m_pScriptEngine->newQMetaObject(&IconAlignment::staticMetaObject);
    m_pScriptEngine->globalObject().setProperty("IconAlignment", enumIconObjectValue);
    QJSValue enumTextObjectValue = m_pScriptEngine->newQMetaObject(&TextAlignment::staticMetaObject);
    m_pScriptEngine->globalObject().setProperty("TextAlignment", enumTextObjectValue);

    // create wrapper function to make syntax of including scripts easier
    QString sSkript = QString("(function() { "
                              "include = function(resource) { "
                              "   var ret = utils_1337.include(resource); "
                              "   if (typeof ret === 'string') { return eval(ret); } "
                              "   else { return (function(){ return eval(ret); })(); } "
                              "}})();");
    m_pScriptEngine->evaluate(sSkript);
  }

  //--------------------------------------------------------------------------------------
  //
  void Deinit() override
  {
    InterruptExecution();

    ResetEngine();

    if (nullptr != m_pScriptEngine)
    {
      m_pScriptEngine->globalObject().setProperty("utils_1337", QJSValue());
      m_pScriptEngine->collectGarbage();
    }

    delete m_pScriptUtils;
    m_pScriptUtils = nullptr;

    m_pScriptEngine->setUrlInterceptor(nullptr);
    delete m_pUrlInterceptor;
    m_pUrlInterceptor = nullptr;

    if (nullptr != m_pScriptEngine)
    {
      m_pScriptEngine->deleteLater();
    }
  }

  //--------------------------------------------------------------------------------------
  //
  void InterruptExecution() override
  {
    if (nullptr != m_pScriptEngine)
    {
      emit SignalInterruptExecution();
      m_pScriptEngine->setInterrupted(true);
    }
    if (thread() == QThread::currentThread())
    {
      while (m_bRunning) QCoreApplication::processEvents();
    }
    else
    {
      while (m_bRunning) QThread::sleep(1);
    }
  }

  //----------------------------------------------------------------------------------------
  //
  void RunScript(const QString& sScript,
                 tspScene spScene, tspResource spResource) override
  {
    // set scene
    if (nullptr != m_pCurrentScene)
    {
      delete m_pCurrentScene;
    }
    if (nullptr != spScene)
    {
      m_pCurrentScene = new CSceneScriptWrapper(m_pScriptEngine, spScene);
      // we need to change ownership
      QQmlEngine::setObjectOwnership(m_pCurrentScene, QQmlEngine::CppOwnership);
      QJSValue sceneValue = m_pScriptEngine->newQObject(m_pCurrentScene);
      m_pScriptEngine->globalObject().setProperty("scene", sceneValue);
    }

    // set current Project
    {
      QReadLocker scriptLocker(&spResource->m_rwLock);
      m_spProject = spResource->m_spParent;
      for (auto it = m_objectMap.begin(); m_objectMap.end() != it; ++it)
      {
        it->second->SetCurrentProject(spResource->m_spParent);
      }
      m_pUrlInterceptor->SetCurrentProject(spResource->m_spParent);
      m_pScriptUtils->SetCurrentProject(spResource->m_spParent);

      if (nullptr == spScene)
      {
        m_pCurrentProject = new CProjectScriptWrapper(m_pScriptEngine, m_spProject);
        // we need to change ownership
        QQmlEngine::setObjectOwnership(m_pCurrentProject, QQmlEngine::CppOwnership);
        QJSValue projectValue = m_pScriptEngine->newQObject(m_pCurrentProject);
        m_pScriptEngine->globalObject().setProperty("project", projectValue);
      }
    }

    // resume engine if interrupetd
    m_pScriptEngine->setInterrupted(false);
    m_bRunning = 1;

    auto spSignalEmitterContext = m_wpSignalEmiterContext.lock();
    if (nullptr == spSignalEmitterContext)
    {
      QJSValue valDummy;
      HandleError(valDummy);
      return;
    }

    spSignalEmitterContext->SetScriptExecutionStatus(CScriptRunnerSignalEmiter::eRunning);

    // get scene name and set it as function, so error messages show the scene name in the error

    QString sSceneName = QString();
    {
      QReadLocker locker(&spResource->m_rwLock);
      sSceneName = QString(spResource->m_sName).replace(QRegExp("\\W"), "_");
    }
    if (nullptr != spScene)
    {
      QReadLocker locker(&spScene->m_rwLock);
      sSceneName = QString(spScene->m_sName).replace(QRegExp("\\W"), "_");
    }

    // create wrapper function to make syntax of scripts easier and handle return value
    // and be able to emit signal on finished
    QString sSkript = QString("(function() { "
                              "var %1 = function() { %2\n}; "
                              "var ret = %3(); "
                              "utils_1337.finishedScript(ret); \n "
                              "})")
        .arg(sSceneName)
        .arg(sScript)
        .arg(sSceneName);
    ReplaceImportStatements(sSkript, ::EvalImportReplacer);
    QJSValue runFunction = m_pScriptEngine->evaluate(sSkript);

    if (runFunction.isError())
    {
      HandleError(runFunction);
      return;
    }

    if (runFunction.isCallable())
    {
      if (!sSceneName.isEmpty())
      {
        emit SignalSceneLoaded(sSceneName);
      }

      QJSValue ret = runFunction.call();
      if (!m_pScriptEngine->isInterrupted())
      {
        if (ret.isError())
        {
          HandleError(ret);
        }
        m_bRunning = 0;
      }
      else
      {
        emit HandleScriptFinish(false, QString());
        m_bRunning = 0;
      }
    }
    else
    {
      QString sError =  tr("Cannot call java-script.");
      qWarning() << sError;
      emit spSignalEmitterContext->showError(sError, QtMsgType::QtCriticalMsg);
      emit spSignalEmitterContext->executionError(sError, 0, "");


      emit HandleScriptFinish(false, QString());
      m_bRunning = 0;
    }
  }

  //--------------------------------------------------------------------------------------
  //
  void RegisterNewComponent(const QString& sName,
                            std::weak_ptr<CScriptCommunicator> wpCommunicator) override
  {
    auto it = m_objectMap.find(sName);
    if (m_objectMap.end() == it)
    {
      if (auto spComm = wpCommunicator.lock())
      {
        std::shared_ptr<CScriptObjectBase> spObject =
            std::shared_ptr<CScriptObjectBase>(
            spComm->CreateNewScriptObject(QPointer<QJSEngine>(m_pScriptEngine)));
        if (nullptr != spObject)
        {
          connect(this, &CJsScriptRunnerInstanceWorker::SignalInterruptExecution,
                  spObject.get(), &CScriptObjectBase::SignalInterruptExecution, Qt::QueuedConnection);

          if (spObject->thread() != thread())
          {
            assert(false && "Thread of Object not correct.");
            qWarning() << tr("Thread of Object %1 not correct.").arg(sName);
            spObject->moveToThread(thread());
          }
          m_objectMap.insert({ sName, spObject });

          if (auto pNotification = dynamic_cast<CScriptNotification*>(spObject.get()))
          {
            connect(pNotification, &CScriptNotification::SignalOverlayCleared,
                    this, [this](){
              emit SignalClearThreads(EScriptRunnerType::eOverlay);
            });
            connect(pNotification, &CScriptNotification::SignalOverlayClosed,
                    this, &CJsScriptRunnerInstanceWorker::SignalKill);
            connect(pNotification, &CScriptNotification::SignalOverlayRunAsync,
                    this, [this](const QString& sId, const QString& sScriptResource){
              emit SignalRunAsync(m_spProject, sId, sScriptResource,
                                  EScriptRunnerType::eOverlay);
            });
          }
          else if (auto pThread = dynamic_cast<CScriptThread*>(spObject.get()))
          {
            connect(pThread, &CScriptThread::SignalKill,
                    this, &CJsScriptRunnerInstanceWorker::SignalKill);
            connect(pThread, &CScriptThread::SignalOverlayRunAsync,
                    this, [this](const QString& sId, const QString& sScriptResource){
              emit SignalRunAsync(m_spProject, sId, sScriptResource,
                                  EScriptRunnerType::eAsync);
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
  void ResetEngine() override
  {
    // remove ref to run function
    m_pScriptEngine->globalObject().setProperty("scene", QJSValue());
    m_pScriptEngine->globalObject().setProperty("project", QJSValue());

    m_pUrlInterceptor->SetCurrentProject(nullptr);
    m_pScriptUtils->SetCurrentProject(nullptr);

    for (auto it = m_objectMap.begin(); m_objectMap.end() != it; ++it)
    {
      m_pScriptEngine->globalObject().setProperty(it->first, QJSValue());
      it->second->Cleanup();
      it->second->SetCurrentProject(nullptr);
    }

    for (const QString& sToDelete : m_vsObjectToDeleteMap)
    {
      auto it = m_objectMap.find(sToDelete);
      if (m_objectMap.end() != it)
      {
        m_objectMap.erase(it);
      }
    }

    m_vsObjectToDeleteMap.clear();

    m_pScriptEngine->collectGarbage();

    if (nullptr != m_pCurrentScene)
    {
      delete m_pCurrentScene;
    }
    if (nullptr != m_pCurrentProject)
    {
      delete m_pCurrentProject;
    }
  }

  //--------------------------------------------------------------------------------------
  //
  void UnregisterComponent(const QString& sName) override
  {
    auto it = std::find(m_vsObjectToDeleteMap.begin(), m_vsObjectToDeleteMap.end(), sName);
    if (m_vsObjectToDeleteMap.end() == it)
    {
      m_vsObjectToDeleteMap.push_back(sName);
    }
  }

private:
  tspProject                                     m_spProject;
  QPointer<QQmlEngine>                           m_pScriptEngine;
  QPointer<CScriptRunnerUtilsJs>                 m_pScriptUtils;
  CResourceUrlInterceptor*                       m_pUrlInterceptor;
  QPointer<CSceneScriptWrapper>                  m_pCurrentScene;
  QPointer<CProjectScriptWrapper>                m_pCurrentProject;
  std::map<QString /*name*/,
           std::shared_ptr<CScriptObjectBase>>   m_objectMap;
  std::vector<QString>                           m_vsObjectToDeleteMap;
  QString                                        m_sName;
};

//----------------------------------------------------------------------------------------
//
CJsScriptRunner::CJsScriptRunner(std::weak_ptr<CScriptRunnerSignalContext> spSignalEmitterContext,
                                 tRunningScriptsCheck fnRunningScriptsCheck,
                                 QObject* pParent) :
  QObject(pParent),
  IScriptRunnerFactory(),
  m_wpSignalEmitterContext(spSignalEmitterContext),
  m_communicatorMutex(QMutex::Recursive),
  m_wpCommunicators(),
  m_fnRunningScriptsCheck(fnRunningScriptsCheck)
{
}

CJsScriptRunner::~CJsScriptRunner()
{
}

//----------------------------------------------------------------------------------------
//
void CJsScriptRunner::Initialize()
{
}

//----------------------------------------------------------------------------------------
//
void CJsScriptRunner::Deinitialize()
{
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<IScriptRunnerInstanceController>
CJsScriptRunner::LoadScript(const QString& sScript,
                            tspScene spScene, tspResource spResource)
{
  std::shared_ptr<CScriptRunnerInstanceController> spRunner = CreateRunner(c_sMainRunner);
  RunScript(spRunner, sScript, spScene, spResource);
  return spRunner;
}

//----------------------------------------------------------------------------------------
//
void CJsScriptRunner::RegisterNewComponent(const QString& sName,
                                           std::weak_ptr<CScriptCommunicator> wpCommunicator)
{
  auto spComm = wpCommunicator.lock();
  assert(nullptr != spComm && "Communicator is null in JS runner");
  if (spComm == nullptr)
  {
    qWarning() << tr("Communicator is null in JS runner");
    return;
  }

  QMutexLocker lockerEmiter(&m_communicatorMutex);
  m_wpCommunicators.insert({sName, wpCommunicator});
}

//----------------------------------------------------------------------------------------
//
void CJsScriptRunner::UnregisterComponent(const QString& sName)
{
  QMutexLocker lockerEmiter(&m_communicatorMutex);
  auto it = m_wpCommunicators.find(sName);
  if (m_wpCommunicators.end() != it)
  {
    m_wpCommunicators.erase(it);
  }
  else
  {
    if (Q_UNLIKELY(!m_wpCommunicators.empty()))
    {
      qWarning() << tr("Communicator %1 not found in JS runner.").arg(sName);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CJsScriptRunner::UnregisterComponents()
{
  QMutexLocker lockerEmiter(&m_communicatorMutex);
  m_wpCommunicators.clear();
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<IScriptRunnerInstanceController> CJsScriptRunner::RunAsync(
    const QString& sId, const QString& sScript, tspResource spResource)
{
  std::shared_ptr<CScriptRunnerInstanceController> spController = CreateRunner(sId);

  auto spSignalEmitterContext = m_wpSignalEmitterContext.lock();
  if (nullptr == spSignalEmitterContext)
  {
    qWarning() << "m_wpSignalEmitterContext was null";
    return nullptr;
  }

  if (nullptr == spResource ||
      nullptr == spResource->m_spParent)
  {
    QString sError = tr("Script file, Scene or Project is null");
    qWarning() << sError;
    emit spSignalEmitterContext->showError(sError, QtMsgType::QtCriticalMsg);
    return nullptr;
  }

  RunScript(spController, sScript, nullptr, spResource);
  return spController;
}

//----------------------------------------------------------------------------------------
//
void CJsScriptRunner::SlotHandleScriptFinish(const QString& sName, bool bSuccess,
                                             const QVariant& sRetVal)
{
  emit SignalRemoveScriptRunner(sName);

  if (!bSuccess)
  {
    qWarning() << tr("Error in script or debugger closed, unloading project.");
  }

  if (c_sMainRunner == sName || QVariant::String == sRetVal.type() || !m_fnRunningScriptsCheck())
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
std::shared_ptr<CScriptRunnerInstanceController> CJsScriptRunner::CreateRunner(const QString& sId)
{
  std::shared_ptr<CScriptRunnerInstanceController> spController =
      std::make_shared<CScriptRunnerInstanceController>(
                          sId,
                          std::make_shared<CJsScriptRunnerInstanceWorker>(
                            sId, m_wpSignalEmitterContext),
                          m_wpSignalEmitterContext);
  spController->setObjectName(sId);
  connect(spController.get(), &CScriptRunnerInstanceController::HandleScriptFinish,
          this, &CJsScriptRunner::SlotHandleScriptFinish);
  connect(spController.get(), &CScriptRunnerInstanceController::SignalSceneLoaded,
          this, &CJsScriptRunner::SignalSceneLoaded);

  connect(spController.get(), &CScriptRunnerInstanceController::SignalClearThreads,
          this, &CJsScriptRunner::SignalClearThreads);
  connect(spController.get(), &CScriptRunnerInstanceController::SignalKill,
          this, &CJsScriptRunner::SignalKill);
  connect(spController.get(), &CScriptRunnerInstanceController::SignalRunAsync,
          this, &CJsScriptRunner::SignalRunAsync);

  QMutexLocker lockerEmiter(&m_communicatorMutex);
  for (auto& itComm : m_wpCommunicators)
  {
    spController->RegisterNewComponent(itComm.first, itComm.second);
  }
  return spController;
}

//----------------------------------------------------------------------------------------
//
void CJsScriptRunner::RunScript(std::shared_ptr<CScriptRunnerInstanceController> spController,
                                const QString& sScript,
                                tspScene spScene, tspResource spResource)
{
  spController->RunScript(sScript, spScene, spResource);
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CScriptRunnerSignalContext> CJsScriptRunner::SignalEmmitterContext()
{
  return m_wpSignalEmitterContext.lock();
}

//----------------------------------------------------------------------------------------
//
CResourceUrlInterceptor::CResourceUrlInterceptor() :
  QQmlAbstractUrlInterceptor()
{
}
CResourceUrlInterceptor::~CResourceUrlInterceptor()
{
}

//----------------------------------------------------------------------------------------
//
void CResourceUrlInterceptor::SetCurrentProject(tspProject spProject)
{
  m_spProject = spProject;
}

//----------------------------------------------------------------------------------------
//
QUrl CResourceUrlInterceptor::intercept(const QUrl& url, QQmlAbstractUrlInterceptor::DataType type)
{
  if (nullptr != m_spProject)
  {
    switch (type)
    {
      case QQmlAbstractUrlInterceptor::DataType::JavaScriptFile: [[fallthrough]];
      case QQmlAbstractUrlInterceptor::DataType::UrlString:
        if (CPhysFsFileEngineHandler::c_sScheme.contains(url.scheme()))
        {
          return PhysicalResourcePath(url, m_spProject);
        }
        return url;
      case QQmlAbstractUrlInterceptor::DataType::QmlFile:
        break;
      case QQmlAbstractUrlInterceptor::DataType::QmldirFile:
        break;
    }
  }
  return url;
}

//----------------------------------------------------------------------------------------
//
CScriptRunnerUtilsJs::CScriptRunnerUtilsJs(QObject* pParent,
                                           QPointer<QQmlEngine> pEngine,
                                           std::shared_ptr<CScriptRunnerSignalContext> spSignalEmiterContext) :
  QObject(pParent),
  m_spSignalEmiterContext(spSignalEmiterContext),
  m_pEngine(pEngine)
{

}
CScriptRunnerUtilsJs::~CScriptRunnerUtilsJs()
{

}

//----------------------------------------------------------------------------------------
//
void CScriptRunnerUtilsJs::SetCurrentProject(tspProject spProject)
{
  m_spProject = spProject;
}

//----------------------------------------------------------------------------------------
//
QJSValue CScriptRunnerUtilsJs::include(QJSValue resource)
{
  tspResource spResource = GetResource(resource);
  if (nullptr != spResource)
  {
    QReadLocker locker(&spResource->m_rwLock);
    if (EResourceType::eScript == spResource->m_type._to_integral())
    {
      QString sPath = ResourceUrlToAbsolutePath(spResource);
      QFile sciptFile(sPath);
      if (sciptFile.exists() && sciptFile.open(QIODevice::ReadOnly))
      {
        QString sScript = QString::fromUtf8(sciptFile.readAll());
        ReplaceImportStatements(sScript, ::EvalImportReplacer);
        if (sScript.startsWith("{") && sScript.endsWith("}"))
        {
          return m_pEngine->evaluate("(" + sScript + ")", spResource->m_sName);
        }
        else
        {
          return sScript;
        }
      }
    }
  }
  return QJSValue();
}

//----------------------------------------------------------------------------------------
//
QJSValue CScriptRunnerUtilsJs::import(QJSValue resource)
{
  tspResource spResource = GetResource(resource);
  if (nullptr != spResource)
  {
    QReadLocker locker(&spResource->m_rwLock);
    if (EResourceType::eScript == spResource->m_type._to_integral())
    {
      QString sPath = ResourceUrlToAbsolutePath(spResource);
      QFile fModule(sPath);
      if (sPath.isEmpty() || !fModule.exists())
      {
        m_pEngine->throwError(tr("Module %1 is missing.").arg(spResource->m_sName));
        return QJSValue();
      }
      if (!fModule.open(QIODevice::ReadOnly))
      {
        m_pEngine->throwError(tr("Module %1 could not be opened.").arg(spResource->m_sName));
        return QJSValue();
      }
      QString sScript = QString::fromUtf8(fModule.readAll());
      ReplaceImportStatements(sScript, ::EvalImportReplacer);
      qint32 iProjId = -1;
      {
        QReadLocker l(&m_spProject->m_rwLock);
        iProjId = m_spProject->m_iId;
      }
      CScriptCacheFileEngineHandler::RegisterFile(iProjId, spResource->m_sName, sScript);
      return m_pEngine->importModule(CScriptCacheFileEngineHandler::c_sScheme +
                                     QString::number(iProjId) + "/" + spResource->m_sName);
    }
  }
  return QJSValue();
}

//----------------------------------------------------------------------------------------
//
tspResource CScriptRunnerUtilsJs::GetResource(QJSValue resource)
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
  return spResource;
}

#include "JsScriptRunner.moc"
