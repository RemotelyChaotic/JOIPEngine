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

CScriptRunner::CScriptRunner() :
  CSystemBase(),
  m_spSettings(CApplication::Instance()->Settings()),
  m_spScriptEngine(std::make_unique<QJSEngine>()),
  m_spSignalEmitter(std::make_shared<CScriptRunnerSignalEmiter>()),
  m_spTimer(nullptr),
  m_objectMap(),
  m_runFunction()
{
  qRegisterMetaType<QColor>();
  qRegisterMetaType<std::vector<QColor>>();
  qRegisterMetaType<QStringList>();

  qRegisterMetaType<CResource*>();
  qRegisterMetaType<tspResource>();
  qRegisterMetaType<CScene*>();
  qRegisterMetaType<tspScene>();
  qRegisterMetaType<CProject*>();
  qRegisterMetaType<tspProject>();

  m_spScriptEngine->installExtensions(QJSEngine::TranslationExtension |
                                      QJSEngine::ConsoleExtension |
                                      QJSEngine::GarbageCollectionExtension);
}

CScriptRunner::~CScriptRunner()
{

}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CScriptRunnerSignalEmiter> CScriptRunner::SignalEmmitter()
{
  return m_spSignalEmitter;
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::Initialize()
{
  m_spTimer = std::make_shared<QTimer>();
  connect(m_spTimer.get(), &QTimer::timeout, this, &CScriptRunner::SlotRun);

  m_objectMap.insert({"background", std::make_shared<CScriptBackground>(m_spSignalEmitter, m_spScriptEngine.get()) });
  m_objectMap.insert({"mediaPlayer", std::make_shared<CScriptMediaPlayer>(m_spSignalEmitter, m_spScriptEngine.get()) });
  m_objectMap.insert({"localStorage", std::make_shared<CScriptStorage>(m_spSignalEmitter, m_spScriptEngine.get()) });
  m_objectMap.insert({"textBox", std::make_shared<CScriptTextBox>(m_spSignalEmitter, m_spScriptEngine.get()) });
  m_objectMap.insert({"timer", std::make_shared<CScriptTimer>(m_spSignalEmitter, m_spScriptEngine.get()) });
  m_objectMap.insert({"thread", std::make_shared<CScriptThread>(m_spSignalEmitter, m_spScriptEngine.get()) });
  m_objectMap.insert({"icon", std::make_shared<CScriptIcon>(m_spSignalEmitter, m_spScriptEngine.get()) });

  for (auto it = m_objectMap.begin(); m_objectMap.end() != it; ++it)
  {
    QJSValue scriptValue = m_spScriptEngine->newQObject(it->second.get());
    m_spScriptEngine->globalObject().setProperty(it->first, scriptValue);
  }

  SetInitialized(true);
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::Deinitialize()
{
  m_spTimer->stop();
  m_spTimer = nullptr;

  for (auto it = m_objectMap.begin(); m_objectMap.end() != it; ++it)
  {
    it->second->Cleanup();
  }

  m_spScriptEngine->collectGarbage();

  m_objectMap.clear();

  SetInitialized(false);
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::SlotLoadScript(tspScene spScene, tspResource spResource)
{
  if (nullptr == spResource || nullptr == spScene ||
      nullptr == spResource->m_spParent)
  {
    QString sError = tr("Script file, Scene or Project is null");
    qCritical() << sError;
    emit m_spSignalEmitter->SignalShowError(sError, QtMsgType::QtCriticalMsg);
    return;
  }

  QReadLocker projectLocker(&spResource->m_spParent->m_rwLock);
  QReadLocker resourceLocker(&spResource->m_rwLock);
  if (spResource->m_type._to_integral() != EResourceType::eOther)
  {
    QString sError = tr("Script resource is of wrong type.");
    qCritical() << sError;
    emit m_spSignalEmitter->SignalShowError(sError, QtMsgType::QtCriticalMsg);
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
      for (auto it = m_objectMap.begin(); m_objectMap.end() != it; ++it)
      {
        it->second->SetCurrentProject(spResource->m_spParent);
      }

      m_spSignalEmitter->SetScriptExecutionStatus(EScriptExecutionStatus::eRunning);

      m_runFunction = m_spScriptEngine->evaluate(
        QString("(function() { %1 \n })").arg(sScript));

      m_spTimer->setSingleShot(true);
      m_spTimer->start(10);
    }
    else
    {
      QString sError = tr("Script resource file could not be opened.");
      qCritical() << sError;
      emit m_spSignalEmitter->SignalShowError(sError, QtMsgType::QtCriticalMsg);
      return;
    }
  }
  else
  {
    QString sError = tr("Script resource file does not exist.");
    qCritical() << sError;
    emit m_spSignalEmitter->SignalShowError(sError, QtMsgType::QtCriticalMsg);
    return;
  }
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
      emit m_spSignalEmitter->SignalShowError(sError, QtMsgType::QtCriticalMsg);
      emit m_spSignalEmitter->SignalExecutionError(sException, iLineNr, sStack);
      return;
    }

    emit m_spSignalEmitter->SignalInterruptLoops();

    for (auto it = m_objectMap.begin(); m_objectMap.end() != it; ++it)
    {
      it->second->SetCurrentProject(nullptr);
    }

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

    emit m_spSignalEmitter->SignalInterruptLoops();

    for (auto it = m_objectMap.begin(); m_objectMap.end() != it; ++it)
    {
      it->second->SetCurrentProject(nullptr);
    }

    emit SignalScriptRunFinished(false, QString());
  }
}
