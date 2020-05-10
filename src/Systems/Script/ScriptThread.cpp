#include "ScriptThread.h"

#include <QEventLoop>
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
    QTimer timer;
    timer.setSingleShot(true);
    timer.setInterval(iTimeS * 1000);
    QEventLoop loop;
    if (bSkippableFlag)
    {
      connect(pSignalEmitter, &CThreadSignalEmitter::waitSkipped,
              &loop, &QEventLoop::quit, Qt::QueuedConnection);
    }
    connect(pSignalEmitter, &CThreadSignalEmitter::interrupt,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    if (bSkippableFlag)
    {
      emit pSignalEmitter->skippableWait(iTimeS);
    }
    timer.start();
    loop.exec();
    timer.disconnect();
    loop.disconnect();
  }
}
