#include "ScriptThread.h"

#include <QEventLoop>
#include <QDateTime>
#include <QThread>
#include <QTimer>

CThreadSignalEmitter::CThreadSignalEmitter() :
  CScriptRunnerSignalEmiter()
{

}
CThreadSignalEmitter::~CThreadSignalEmitter()
{

}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CScriptObjectBase> CThreadSignalEmitter::CreateNewScriptObject(QPointer<QJSEngine> pEngine)
{
  return std::make_shared<CScriptThread>(this, pEngine);
}

//----------------------------------------------------------------------------------------
//
CScriptThread::CScriptThread(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                             QPointer<QJSEngine> pEngine) :
  CScriptObjectBase(pEmitter, pEngine)
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

  auto pSignalEmitter = SignalEmitter<CThreadSignalEmitter>();

  bool bSkippableFlag = false;
  if (bSkippable.isBool())
  {
    bSkippableFlag = bSkippable.toBool();
  }

  if (0 < iTimeS)
  {
    QDateTime lastTime = QDateTime::currentDateTime();
    qint32 iTimeLeft = iTimeS * 1000;

    QTimer timer;
    timer.setSingleShot(false);
    timer.setInterval(20);
    QEventLoop loop;
    connect(pSignalEmitter, &CThreadSignalEmitter::interrupt,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);

    // connect lambdas in loop context, so events are processed, but capture timer,
    // to start / stop
    connect(pSignalEmitter, &CThreadSignalEmitter::pauseExecution, &loop, [&timer]() {
      timer.stop();
    }, Qt::QueuedConnection);
    connect(pSignalEmitter, &CThreadSignalEmitter::resumeExecution, &loop, [&timer]() {
      timer.start();
    }, Qt::QueuedConnection);

    connect(&timer, &QTimer::timeout, &loop, [&loop, &iTimeLeft, &lastTime]() {
      QDateTime newTime = QDateTime::currentDateTime();
      iTimeLeft -= newTime.toMSecsSinceEpoch() - lastTime.toMSecsSinceEpoch();
      lastTime = newTime;
      if (0 >= iTimeLeft)
      {
        emit loop.exit();
      }
    });

    if (bSkippableFlag)
    {
      connect(pSignalEmitter, &CThreadSignalEmitter::waitSkipped,
              &loop, &QEventLoop::quit, Qt::QueuedConnection);
      emit pSignalEmitter->skippableWait(iTimeS);
    }
    timer.start();
    loop.exec();
    timer.stop();
    timer.disconnect();
    loop.disconnect();
  }
}
