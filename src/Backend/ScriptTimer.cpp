#include "ScriptTimer.h"
#include "ScriptRunnerSignalEmiter.h"

#include <QEventLoop>

CScriptTimer::CScriptTimer(std::shared_ptr<CScriptRunnerSignalEmiter> spEmitter,
                           QJSEngine* pEngine) :
  QObject(),
  m_spSignalEmitter(spEmitter),
  m_pEngine(pEngine)
{

}

CScriptTimer::~CScriptTimer()
{

}

//----------------------------------------------------------------------------------------
//
void CScriptTimer::hide()
{
  emit m_spSignalEmitter->SignalHideTimer();
}

//----------------------------------------------------------------------------------------
//
void CScriptTimer::setTime(qint32 iTimeS)
{
  emit m_spSignalEmitter->SignalSetTime(iTimeS);
}

//----------------------------------------------------------------------------------------
//
void CScriptTimer::setTimeVisible(bool bVisible)
{
  emit m_spSignalEmitter->SignalSetTimeVisible(bVisible);
}

//----------------------------------------------------------------------------------------
//
void CScriptTimer::show()
{
  emit m_spSignalEmitter->SignalShowTimer();
}

//----------------------------------------------------------------------------------------
//
void CScriptTimer::start()
{
  emit m_spSignalEmitter->SignalStartTimer();
}

//----------------------------------------------------------------------------------------
//
void CScriptTimer::stop()
{
  emit m_spSignalEmitter->SignalStopTimer();
}

//----------------------------------------------------------------------------------------
//
void CScriptTimer::waitForTimer()
{
  QEventLoop loop;
  connect(m_spSignalEmitter.get(), &CScriptRunnerSignalEmiter::SignalTimerFinished,
          &loop, &QEventLoop::quit, Qt::QueuedConnection);
  connect(m_spSignalEmitter.get(), &CScriptRunnerSignalEmiter::SignalInterruptLoops,
          &loop, &QEventLoop::quit, Qt::QueuedConnection);
  emit m_spSignalEmitter->SignalWaitForTimer();
  loop.exec();
}
