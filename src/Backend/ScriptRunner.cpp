#include "ScriptRunner.h"
#include "Application.h"
#include "Settings.h"

CScriptRunner::CScriptRunner() :
  CThreadedObject(),
  m_spSettings(CApplication::Instance()->Settings()),
  m_spScriptEngine(std::make_unique<QScriptEngine>())
{

}

CScriptRunner::~CScriptRunner()
{

}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::Initialize()
{

  SetInitialized(true);
}

//----------------------------------------------------------------------------------------
//
void CScriptRunner::Deinitialize()
{

  SetInitialized(false);
}
