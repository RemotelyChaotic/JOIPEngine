#include "ScriptThread.h"
#include "Application.h"
#include "CommonScriptHelpers.h"
#include "IScriptRunner.h"

#include "Systems/DatabaseManager.h"

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
std::shared_ptr<CScriptObjectBase> CThreadSignalEmitter::CreateNewScriptObject(QtLua::State* pState)
{
  return std::make_shared<CScriptThread>(this, pState);
}
std::shared_ptr<CScriptObjectBase> CThreadSignalEmitter::CreateNewSequenceObject()
{
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
CScriptThread::CScriptThread(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                             QPointer<QJSEngine> pEngine) :
  CJsScriptObjectBase(pEmitter, pEngine),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
{
}
CScriptThread::CScriptThread(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                             QtLua::State* pState)  :
  CJsScriptObjectBase(pEmitter, pState),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
{
}

CScriptThread::~CScriptThread()
{

}

//----------------------------------------------------------------------------------------
//
void CScriptThread::kill(QString sId)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (IScriptRunnerFactory::c_sMainRunner == sId)
  {
    emit m_pSignalEmitter->showError(
          tr("Can not kill %1.").arg(IScriptRunnerFactory::c_sMainRunner),
          QtMsgType::QtWarningMsg);
    return;
  }

  emit SignalKill(sId);
}

//----------------------------------------------------------------------------------------
//
void CScriptThread::runAsynch(QVariant resource)
{
  runAsynch(resource, QVariant());
}

//----------------------------------------------------------------------------------------
//
void CScriptThread::runAsynch(QVariant resource, QVariant id)
{
  if (!CheckIfScriptCanRun()) { return; }

  QString sId;
  if (id.type() == QVariant::String)
  {
    sId = id.toString();
  }

  bool bOk = false;
  QString sResource = GetResourceName(resource, "runAsynch", false, &bOk);
  if (!bOk) { return; }

  if (sId.isEmpty())
  {
    sId = sResource;
  }

  if (IScriptRunnerFactory::c_sMainRunner == sId)
  {
    emit m_pSignalEmitter->showError(
          tr("Id of asynch thread must not be %1").arg(IScriptRunnerFactory::c_sMainRunner),
          QtMsgType::QtWarningMsg);
    return;
  }

  emit SignalOverlayRunAsync(sId, sResource);
}

//----------------------------------------------------------------------------------------
//
void CScriptThread::sleep(qint32 iTimeS)
{
  sleep(iTimeS, false);
}

//----------------------------------------------------------------------------------------
//
void CScriptThread::sleep(qint32 iTimeS, QVariant bSkippable)
{
  if (!CheckIfScriptCanRun()) { return; }

  auto pSignalEmitter = SignalEmitter<CThreadSignalEmitter>();

  bool bSkippableFlag = false;
  if (bSkippable.type() == QVariant::Bool)
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
    QMetaObject::Connection interruptLoop =
      connect(pSignalEmitter, &CThreadSignalEmitter::interrupt,
              &loop, &QEventLoop::quit, Qt::QueuedConnection);
    QMetaObject::Connection interruptThisLoop =
      connect(this, &CScriptObjectBase::SignalInterruptExecution,
              &loop, [&loop]{
      loop.quit();
    }, Qt::QueuedConnection);

    // connect lambdas in loop context, so events are processed, but capture timer,
    // to start / stop
    QMetaObject::Connection pauseLoop =
      connect(pSignalEmitter, &CThreadSignalEmitter::pauseExecution, &loop, [&timer]() {
        timer.stop();
      }, Qt::QueuedConnection);
    QMetaObject::Connection resumeLoop =
      connect(pSignalEmitter, &CThreadSignalEmitter::resumeExecution, &loop, [&timer]() {
        timer.start();
      }, Qt::QueuedConnection);

    QMetaObject::Connection timeoutLoop =
      connect(&timer, &QTimer::timeout, &loop, [&loop, &iTimeLeft, &lastTime]() {
        QDateTime newTime = QDateTime::currentDateTime();
        iTimeLeft -= newTime.toMSecsSinceEpoch() - lastTime.toMSecsSinceEpoch();
        lastTime = newTime;
        if (0 >= iTimeLeft)
        {
          emit loop.exit();
        }
      });

    QMetaObject::Connection skipLoop;
    if (bSkippableFlag)
    {
      skipLoop =
        connect(pSignalEmitter, &CThreadSignalEmitter::waitSkipped,
                &loop, &QEventLoop::quit, Qt::QueuedConnection);
    }
    emit pSignalEmitter->skippableWait(bSkippableFlag ? iTimeS : 0);

    timer.start();
    loop.exec();
    timer.stop();
    timer.disconnect();
    loop.disconnect();

    disconnect(interruptLoop);
    disconnect(interruptThisLoop);
    disconnect(pauseLoop);
    disconnect(resumeLoop);
    disconnect(timeoutLoop);
    if (bSkippableFlag)
    {
      disconnect(skipLoop);
    }
  }
}

//----------------------------------------------------------------------------------------
//
QString CScriptThread::GetResourceName(const QVariant& resource, const QString& sMethod,
                                       bool bStringCanBeId, bool* pbOk)
{
  QString sError;
  std::optional<QString> optRes =
      script::ParseResourceFromScriptVariant(resource, m_wpDbManager.lock(),
                                             m_spProject,
                                             sMethod, &sError);

  if (nullptr != pbOk)
  {
    *pbOk = optRes.has_value();
  }

  if (optRes.has_value())
  {
    return optRes.value();
  }
  else
  {
    emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
    return QString();
  }
}
