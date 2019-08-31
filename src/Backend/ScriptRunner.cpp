#include "ScriptRunner.h"
#include "Application.h"
#include "ScriptBackground.h"
#include "ScriptIcon.h"
#include "ScriptMediaPlayer.h"
#include "ScriptTextBox.h"
#include "ScriptTimer.h"
#include "ScriptThread.h"
#include "Settings.h"

CScriptRunner::CScriptRunner() :
  CThreadedObject(),
  m_spSettings(CApplication::Instance()->Settings()),
  m_spScriptEngine(std::make_unique<QJSEngine>())
{

}

CScriptRunner::~CScriptRunner()
{

}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::Initialize()
{
  m_spScriptEngine->installExtensions(QJSEngine::TranslationExtension |
                                      QJSEngine::ConsoleExtension |
                                      QJSEngine::GarbageCollectionExtension);

  CScriptBackground* pBackgroundObject = new CScriptBackground(m_spScriptEngine.get());
  QJSValue backgroundValue = m_spScriptEngine->newQObject(pBackgroundObject);
  m_spScriptEngine->globalObject().setProperty("background", backgroundValue);

  CScriptMediaPlayer* pMediaPlayerObject = new CScriptMediaPlayer(m_spScriptEngine.get());
  QJSValue mediaPlayerValue = m_spScriptEngine->newQObject(pMediaPlayerObject);
  m_spScriptEngine->globalObject().setProperty("mediaPlayer", mediaPlayerValue);

  CScriptTextBox* pTextBoxObject = new CScriptTextBox(m_spScriptEngine.get());
  QJSValue textBoxValue = m_spScriptEngine->newQObject(pTextBoxObject);
  m_spScriptEngine->globalObject().setProperty("textBox", textBoxValue);

  CScriptTimer* pTimerObject = new CScriptTimer(m_spScriptEngine.get());
  QJSValue timerValue = m_spScriptEngine->newQObject(pTimerObject);
  m_spScriptEngine->globalObject().setProperty("timer", timerValue);

  CScriptThread* pThreadObject = new CScriptThread(m_spScriptEngine.get());
  QJSValue threadValue = m_spScriptEngine->newQObject(pThreadObject);
  m_spScriptEngine->globalObject().setProperty("thread", threadValue);

  CScriptIcon* pScriptIcon = new CScriptIcon(m_spScriptEngine.get());
  QJSValue iconValue = m_spScriptEngine->newQObject(pScriptIcon);
  m_spScriptEngine->globalObject().setProperty("icon", iconValue);

  SetInitialized(true);
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::Deinitialize()
{

  SetInitialized(false);
}
