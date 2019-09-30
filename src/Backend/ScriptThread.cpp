#include "ScriptThread.h"
#include "ScriptRunnerSignalEmiter.h"

#include <QEventLoop>
#include <QThread>
#include <QTimer>


CScriptThread::CScriptThread(std::shared_ptr<CScriptRunnerSignalEmiter> spEmitter,
                             QJSEngine* pEngine) :
  QObject(),
  m_spSignalEmitter(spEmitter),
  m_pEngine(pEngine)
{

}

CScriptThread::~CScriptThread()
{

}

//----------------------------------------------------------------------------------------
//
void CScriptThread::sleep(qint32 iTimeS, QJSValue bSkippable)
{
  if (!CheckIfScriptCanRun()) { return; }

  bool bSkippableFlag = false;
  if (bSkippable.isBool())
  {
    bSkippableFlag = bSkippable.toBool();
  }

  if (!bSkippableFlag)
  {
    if (0 < iTimeS)
    {
      thread()->sleep(static_cast<unsigned long>(iTimeS));
    }
  }
  else
  {
    QTimer timer;
    timer.setSingleShot(true);
    timer.setInterval(iTimeS * 1000);
    QEventLoop loop;
    connect(m_spSignalEmitter.get(), &CScriptRunnerSignalEmiter::SignalWaitSkipped,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
    connect(m_spSignalEmitter.get(), &CScriptRunnerSignalEmiter::SignalInterruptLoops,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    emit m_spSignalEmitter->SignalSkippableWait(iTimeS);
    timer.start();
    loop.exec();
    timer.disconnect();
    loop.disconnect();
  }
}

//----------------------------------------------------------------------------------------
//
bool CScriptThread::CheckIfScriptCanRun()
{
  if (m_spSignalEmitter->ScriptExecutionStatus()._to_integral() == EScriptExecutionStatus::eStopped)
  {
    QJSValue val = m_pEngine->evaluate("f();"); //undefined function -> create error
    Q_UNUSED(val);
    return false;
  }
  else
  {
    return true;
  }
}
