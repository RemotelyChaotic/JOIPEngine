#include "ScriptThread.h"
#include "ScriptRunnerSignalEmiter.h"

#include <QEventLoop>
#include <QThread>
#include <QTimer>


CScriptThread::CScriptThread(std::shared_ptr<CScriptRunnerSignalEmiter> spEmitter,
                             QPointer<QJSEngine> pEngine) :
  CScriptObjectBase(spEmitter, pEngine)
{

}

CScriptThread::~CScriptThread()
{

}

//----------------------------------------------------------------------------------------
//
void CScriptThread::sleep(qint32 iTimeS)
{
  sleep(iTimeS, false);
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

  if (0 < iTimeS)
  {
    QTimer timer;
    timer.setSingleShot(true);
    timer.setInterval(iTimeS * 1000);
    QEventLoop loop;
    if (bSkippableFlag)
    {
      connect(m_spSignalEmitter.get(), &CScriptRunnerSignalEmiter::SignalWaitSkipped,
              &loop, &QEventLoop::quit, Qt::QueuedConnection);
    }
    connect(m_spSignalEmitter.get(), &CScriptRunnerSignalEmiter::SignalInterruptLoops,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    if (bSkippableFlag)
    {
      emit m_spSignalEmitter->SignalSkippableWait(iTimeS);
    }
    timer.start();
    loop.exec();
    timer.disconnect();
    loop.disconnect();
  }
}
