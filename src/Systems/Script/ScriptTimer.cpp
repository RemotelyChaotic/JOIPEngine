#include "ScriptTimer.h"

#include <QEventLoop>

CTimerSignalEmitter::CTimerSignalEmitter() :
  CScriptRunnerSignalEmiter()
{

}
CTimerSignalEmitter::~CTimerSignalEmitter()
{

}


//----------------------------------------------------------------------------------------
//
std::shared_ptr<CScriptObjectBase> CTimerSignalEmitter::CreateNewScriptObject(QPointer<QJSEngine> pEngine)
{
  return std::make_shared<CScriptTimer>(this, pEngine);
}


//----------------------------------------------------------------------------------------
//
CScriptTimer::CScriptTimer(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                           QPointer<QJSEngine> pEngine) :
  CScriptObjectBase(pEmitter, pEngine)
{

}

CScriptTimer::~CScriptTimer()
{

}

//----------------------------------------------------------------------------------------
//
void CScriptTimer::hide()
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CTimerSignalEmitter>()->hideTimer();
}

//----------------------------------------------------------------------------------------
//
void CScriptTimer::setTime(qint32 iTimeS)
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CTimerSignalEmitter>()->setTime(iTimeS);
}

//----------------------------------------------------------------------------------------
//
void CScriptTimer::setTimeVisible(bool bVisible)
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CTimerSignalEmitter>()->setTimeVisible(bVisible);
}

//----------------------------------------------------------------------------------------
//
void CScriptTimer::show()
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CTimerSignalEmitter>()->showTimer();
}

//----------------------------------------------------------------------------------------
//
void CScriptTimer::start()
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CTimerSignalEmitter>()->startTimer();
}

//----------------------------------------------------------------------------------------
//
void CScriptTimer::stop()
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CTimerSignalEmitter>()->stopTimer();
}

//----------------------------------------------------------------------------------------
//
void CScriptTimer::waitForTimer()
{
  if (!CheckIfScriptCanRun()) { return; }

  auto pSignalEmitter = SignalEmitter<CTimerSignalEmitter>();
  QEventLoop loop;
  connect(pSignalEmitter, &CTimerSignalEmitter::timerFinished,
          &loop, &QEventLoop::quit, Qt::QueuedConnection);
  connect(pSignalEmitter, &CTimerSignalEmitter::interrupt,
          &loop, &QEventLoop::quit, Qt::QueuedConnection);
  emit pSignalEmitter->waitForTimer();
  loop.exec();
  loop.disconnect();
}
