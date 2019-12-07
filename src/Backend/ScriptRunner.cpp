#include "ScriptRunner.h"
#include "Application.h"
#include "Project.h"
#include "Resource.h"
#include "Scene.h"
#include "ScriptBackground.h"
#include "ScriptIcon.h"
#include "ScriptMediaPlayer.h"
#include "ScriptRunnerSignalEmiter.h"
#include "ScriptStorage.h"
#include "ScriptTextBox.h"
#include "ScriptTimer.h"
#include "ScriptThread.h"
#include "Settings.h"

#include <QDebug>
#include <QFileInfo>

CScriptRunner::CScriptRunner() :
  CThreadedObject(),
  m_spSettings(CApplication::Instance()->Settings()),
  m_spScriptEngine(std::make_unique<QJSEngine>()),
  m_spSignalEmitter(std::make_shared<CScriptRunnerSignalEmiter>()),
  m_spTimer(nullptr),
  m_pBackgroundObject(nullptr),
  m_pScriptIcon(nullptr),
  m_pMediaPlayerObject(nullptr),
  m_pStorageObject(nullptr),
  m_pTextBoxObject(nullptr),
  m_pTimerObject(nullptr),
  m_pThreadObject(nullptr),
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

  m_pBackgroundObject = new CScriptBackground(m_spSignalEmitter, m_spScriptEngine.get());
  QJSValue backgroundValue = m_spScriptEngine->newQObject(m_pBackgroundObject);
  m_spScriptEngine->globalObject().setProperty("background", backgroundValue);

  m_pMediaPlayerObject = new CScriptMediaPlayer(m_spSignalEmitter, m_spScriptEngine.get());
  QJSValue mediaPlayerValue = m_spScriptEngine->newQObject(m_pMediaPlayerObject);
  m_spScriptEngine->globalObject().setProperty("mediaPlayer", mediaPlayerValue);

  m_pStorageObject = new CScriptStorage(m_spSignalEmitter, m_spScriptEngine.get());
  QJSValue storageValue = m_spScriptEngine->newQObject(m_pStorageObject);
  m_spScriptEngine->globalObject().setProperty("localStorage", storageValue);

  m_pTextBoxObject = new CScriptTextBox(m_spSignalEmitter, m_spScriptEngine.get());
  QJSValue textBoxValue = m_spScriptEngine->newQObject(m_pTextBoxObject);
  m_spScriptEngine->globalObject().setProperty("textBox", textBoxValue);

  m_pTimerObject = new CScriptTimer(m_spSignalEmitter, m_spScriptEngine.get());
  QJSValue timerValue = m_spScriptEngine->newQObject(m_pTimerObject);
  m_spScriptEngine->globalObject().setProperty("timer", timerValue);

  m_pThreadObject = new CScriptThread(m_spSignalEmitter, m_spScriptEngine.get());
  QJSValue threadValue = m_spScriptEngine->newQObject(m_pThreadObject);
  m_spScriptEngine->globalObject().setProperty("thread", threadValue);

  m_pScriptIcon = new CScriptIcon(m_spSignalEmitter, m_spScriptEngine.get());
  QJSValue iconValue = m_spScriptEngine->newQObject(m_pScriptIcon);
  m_spScriptEngine->globalObject().setProperty("icon", iconValue);

  SetInitialized(true);
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::Deinitialize()
{
  m_spTimer->stop();
  m_spTimer = nullptr;

  m_pStorageObject->ClearStorage();
  m_spScriptEngine->collectGarbage();

  delete m_pBackgroundObject;
  delete m_pScriptIcon;
  delete m_pMediaPlayerObject;
  delete m_pStorageObject;
  delete m_pTextBoxObject;
  delete m_pTimerObject;
  delete m_pThreadObject;

  m_pBackgroundObject = nullptr;
  m_pScriptIcon = nullptr;
  m_pMediaPlayerObject = nullptr;
  m_pStorageObject = nullptr;
  m_pTextBoxObject = nullptr;
  m_pTimerObject = nullptr;
  m_pThreadObject = nullptr;

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
                                            spResource->m_spParent->m_sName);
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
      m_pBackgroundObject->SetCurrentProject(spResource->m_spParent);
      m_pMediaPlayerObject->SetCurrentProject(spResource->m_spParent);
      m_pScriptIcon->SetCurrentProject(spResource->m_spParent);
      m_pTextBoxObject->SetCurrentProject(spResource->m_spParent);

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

    m_pBackgroundObject->SetCurrentProject(nullptr);
    m_pMediaPlayerObject->SetCurrentProject(nullptr);
    m_pScriptIcon->SetCurrentProject(nullptr);
    m_pTextBoxObject->SetCurrentProject(nullptr);

    emit SignalScriptRunFinished(true);
  }
  else
  {
    qDebug() << tr("Cannot call java-script.");

    emit m_spSignalEmitter->SignalInterruptLoops();

    m_pBackgroundObject->SetCurrentProject(nullptr);
    m_pMediaPlayerObject->SetCurrentProject(nullptr);
    m_pScriptIcon->SetCurrentProject(nullptr);
    m_pTextBoxObject->SetCurrentProject(nullptr);

    emit SignalScriptRunFinished(false);
  }
}
