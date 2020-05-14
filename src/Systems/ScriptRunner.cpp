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

namespace {

}

//----------------------------------------------------------------------------------------
//
CScriptRunner::CScriptRunner() :
  CSystemBase(),
  m_spSettings(CApplication::Instance()->Settings()),
  m_spScriptEngine(std::make_unique<QJSEngine>()),
  m_spSignalEmitterContext(std::make_shared<CScriptRunnerSignalContext>()),
  m_spTimer(nullptr),
  m_objectMapMutex(),
  m_objectMap(),
  m_runFunction()
{
  m_spScriptEngine->installExtensions(QJSEngine::TranslationExtension |
                                      QJSEngine::ConsoleExtension |
                                      QJSEngine::GarbageCollectionExtension);
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
  m_spTimer = std::make_shared<QTimer>();
  connect(m_spTimer.get(), &QTimer::timeout, this, &CScriptRunner::SlotRun);

  SetInitialized(true);
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::Deinitialize()
{
  m_spTimer->stop();
  m_spTimer = nullptr;

  unregisterComponents();

  SetInitialized(false);
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::loadScript(tspScene spScene, tspResource spResource)
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
  if (spResource->m_type._to_integral() != EResourceType::eOther)
  {
    QString sError = tr("Script resource is of wrong type.");
    qCritical() << sError;
    emit m_spSignalEmitterContext->showError(sError, QtMsgType::QtCriticalMsg);
    return;
  }

  QString sPath = ResourceUrlToAbsolutePath(spResource->m_sPath,
                                            spResource->m_spParent->m_sFolderName);
  resourceLocker.unlock();
  projectLocker.unlock();

  QFileInfo scriptFileInfo(sPath);
  if (scriptFileInfo.exists() && scriptFileInfo.suffix() == "js")
  {
    QFile scriptFile(sPath);
    if (scriptFile.open(QIODevice::ReadOnly))
    {
      QString sScript = QString::fromUtf8(scriptFile.readAll());

      // set scene
      CScene* pSceneIcon = new CScene(m_spScriptEngine.get(), spScene);
      QJSValue sceneValue = m_spScriptEngine->newQObject(pSceneIcon);
      m_spScriptEngine->globalObject().setProperty("scene", sceneValue);

      // set current Project
      m_objectMapMutex.lock();
      for (auto it = m_objectMap.begin(); m_objectMap.end() != it; ++it)
      {
        it->second->SetCurrentProject(spResource->m_spParent);
      }
      m_objectMapMutex.unlock();

      m_spSignalEmitterContext->SetScriptExecutionStatus(CScriptRunnerSignalEmiter::eRunning);

      m_runFunction = m_spScriptEngine->evaluate(
        QString("(function() { %1 \n })").arg(sScript));

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
void CScriptRunner::registerNewComponent(const QString sName, QJSValue signalEmitter)
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
void CScriptRunner::unregisterComponents()
{
  m_objectMapMutex.lock();
  for (auto it = m_objectMap.begin(); m_objectMap.end() != it; ++it)
  {
    it->second->Cleanup();
  }
  m_objectMapMutex.unlock();

  m_spScriptEngine->collectGarbage();

  m_objectMapMutex.lock();
  m_objectMap.clear();
  m_objectMapMutex.unlock();
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::SlotRun()
{
  if (!m_runFunction.isNull() && m_runFunction.isCallable())
  {
    QJSValue ret = m_runFunction.call();
    if (ret.isError())
    {
      QString sException = ret.property("name").toString();
      qint32 iLineNr = ret.property("lineNumber").toInt() - 1;
      QString sStack = ret.property("stack").toString();
      QString sError = "Uncaught " + sException +
                       " at line" + QString::number(iLineNr) +
                       ":" + ret.toString() + "\n" + sStack;
      qCritical() << sError;
      emit m_spSignalEmitterContext->showError(sError, QtMsgType::QtCriticalMsg);
      emit m_spSignalEmitterContext->executionError(sException, iLineNr, sStack);
      return;
    }

    emit m_spSignalEmitterContext->interrupt();

    m_objectMapMutex.lock();
    for (auto it = m_objectMap.begin(); m_objectMap.end() != it; ++it)
    {
      it->second->SetCurrentProject(nullptr);
    }
    m_objectMapMutex.unlock();

    if (ret.isString())
    {
      emit SignalScriptRunFinished(true, ret.toString());
    }
    else
    {
      emit SignalScriptRunFinished(true, QString());
    }
  }
  else
  {
    qDebug() << tr("Cannot call java-script.");

    emit m_spSignalEmitterContext->interrupt();

    m_objectMapMutex.lock();
    for (auto it = m_objectMap.begin(); m_objectMap.end() != it; ++it)
    {
      it->second->SetCurrentProject(nullptr);
    }
    m_objectMapMutex.unlock();

    emit SignalScriptRunFinished(false, QString());
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
    QJSValue scriptValue = m_spScriptEngine->newQObject(it->second.get());
    m_spScriptEngine->globalObject().setProperty(it->first, scriptValue);
  }
}
