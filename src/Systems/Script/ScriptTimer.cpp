#include "ScriptTimer.h"
#include "ScriptRunnerSignalEmiter.h"

#include <QEventLoop>

CScriptTimer::CScriptTimer(std::shared_ptr<CScriptRunnerSignalEmiter> spEmitter,
                           QPointer<QJSEngine> pEngine) :
  CScriptObjectBase(spEmitter, pEngine)
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
  emit m_spSignalEmitter->SignalHideTimer();
}

//----------------------------------------------------------------------------------------
//
void CScriptTimer::setTime(qint32 iTimeS)
{
  if (!CheckIfScriptCanRun()) { return; }
  emit m_spSignalEmitter->SignalSetTime(iTimeS);
}

//----------------------------------------------------------------------------------------
//
void CScriptTimer::setTimeVisible(bool bVisible)
{
  if (!CheckIfScriptCanRun()) { return; }
  emit m_spSignalEmitter->SignalSetTimeVisible(bVisible);
}

//----------------------------------------------------------------------------------------
//
void CScriptTimer::show()
{
  if (!CheckIfScriptCanRun()) { return; }
  emit m_spSignalEmitter->SignalShowTimer();
}

//----------------------------------------------------------------------------------------
//
void CScriptTimer::start()
{
  if (!CheckIfScriptCanRun()) { return; }
  emit m_spSignalEmitter->SignalStartTimer();
}

//----------------------------------------------------------------------------------------
//
void CScriptTimer::stop()
{
  if (!CheckIfScriptCanRun()) { return; }
  emit m_spSignalEmitter->SignalStopTimer();
}

//----------------------------------------------------------------------------------------
//
void CScriptTimer::waitForTimer()
{
  if (!CheckIfScriptCanRun()) { return; }

  QEventLoop loop;
  connect(m_spSignalEmitter.get(), &CScriptRunnerSignalEmiter::SignalTimerFinished,
          &loop, &QEventLoop::quit, Qt::QueuedConnection);
  connect(m_spSignalEmitter.get(), &CScriptRunnerSignalEmiter::SignalInterruptLoops,
          &loop, &QEventLoop::quit, Qt::QueuedConnection);
  emit m_spSignalEmitter->SignalWaitForTimer();
  loop.exec();
  loop.disconnect();
}
