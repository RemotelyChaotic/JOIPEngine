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
std::shared_ptr<CScriptCommunicator>
CThreadSignalEmitter::CreateCommunicatorImpl(std::shared_ptr<CScriptRunnerSignalEmiterAccessor> spAccessor)
{
  return std::make_shared<CThreadScriptCommunicator>(spAccessor);
}

//----------------------------------------------------------------------------------------
//
CThreadScriptCommunicator::CThreadScriptCommunicator(
  const std::weak_ptr<CScriptRunnerSignalEmiterAccessor>& spEmitter) :
  CScriptCommunicator(spEmitter)
{}
CThreadScriptCommunicator::~CThreadScriptCommunicator() = default;

//----------------------------------------------------------------------------------------
//
CScriptObjectBase* CThreadScriptCommunicator::CreateNewScriptObject(QPointer<QJSEngine> pEngine)
{
  return new CScriptThread(weak_from_this(), pEngine);
}
CScriptObjectBase* CThreadScriptCommunicator::CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser)
{
  Q_UNUSED(pParser)
  return nullptr;
}
CScriptObjectBase* CThreadScriptCommunicator::CreateNewScriptObject(QtLua::State* pState)
{
  return new CScriptThread(weak_from_this(), pState);
}
CScriptObjectBase* CThreadScriptCommunicator::CreateNewSequenceObject()
{
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
CScriptThread::CScriptThread(std::weak_ptr<CScriptCommunicator> pCommunicator,
                             QPointer<QJSEngine> pEngine) :
  CJsScriptObjectBase(pCommunicator, pEngine),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
{
  m_spStop = std::make_shared<std::function<void()>>([this]() {
    emit SignalQuitLoop();
  });
  m_spPause = std::make_shared<std::function<void()>>([this]() {
    emit SignalPauseTimer();
  });
  m_spResume = std::make_shared<std::function<void()>>([this]() {
    emit SignalResumeTimer();
  });
  if (auto spComm = pCommunicator.lock())
  {
    spComm->RegisterStopCallback(m_spStop);
    spComm->RegisterPauseCallback(m_spPause);
    spComm->RegisterResumeCallback(m_spResume);
  }
}
CScriptThread::CScriptThread(std::weak_ptr<CScriptCommunicator> pCommunicator,
                             QtLua::State* pState)  :
  CJsScriptObjectBase(pCommunicator, pState),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
{
  m_spStop = std::make_shared<std::function<void()>>([this]() {
    emit SignalQuitLoop();
  });
  m_spPause = std::make_shared<std::function<void()>>([this]() {
    emit SignalPauseTimer();
  });
  m_spResume = std::make_shared<std::function<void()>>([this]() {
    emit SignalResumeTimer();
  });
  if (auto spComm = pCommunicator.lock())
  {
    spComm->RegisterStopCallback(m_spStop);
    spComm->RegisterPauseCallback(m_spPause);
    spComm->RegisterResumeCallback(m_spResume);
  }
}

CScriptThread::~CScriptThread()
{
  if (auto spComm = m_wpCommunicator.lock())
  {
    spComm->RemoveResumeCallback(m_spResume);
    spComm->RemovePauseCallback(m_spPause);
    spComm->RemoveStopCallback(m_spStop);
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptThread::kill(QString sId)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CThreadSignalEmitter>())
    {
      if (IScriptRunnerFactory::c_sMainRunner == sId)
      {
        emit spSignalEmitter->showError(
              tr("Can not kill %1.").arg(IScriptRunnerFactory::c_sMainRunner),
              QtMsgType::QtWarningMsg);
        return;
      }
    }
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
    if (auto spComm = m_wpCommunicator.lock())
    {
      if (auto spSignalEmitter = spComm->LockedEmitter<CThreadSignalEmitter>())
      {
        emit spSignalEmitter->showError(
              tr("Id of asynch thread must not be %1").arg(IScriptRunnerFactory::c_sMainRunner),
              QtMsgType::QtWarningMsg);
      }
    }
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

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CThreadSignalEmitter>())
    {
      bool bSkippableFlag = false;
      if (bSkippable.type() == QVariant::Bool)
      {
        bSkippableFlag = bSkippable.toBool();
      }

      if (0 < iTimeS)
      {
        QTimer::singleShot(0, this, [this, bSkippableFlag, iTimeS]() {
          if (auto spComm = m_wpCommunicator.lock())
          {
            if (auto spSignalEmitter = spComm->LockedEmitter<CThreadSignalEmitter>())
            {
              emit spSignalEmitter->skippableWait(bSkippableFlag ? iTimeS : 0);
            }
          }
        });

        QDateTime lastTime = QDateTime::currentDateTime();
        qint32 iTimeLeft = iTimeS * 1000;

        QPointer<CScriptThread> pThis(this);
        QTimer timer;
        timer.setSingleShot(false);
        timer.setInterval(20);
        QEventLoop loop;

        QMetaObject::Connection quitLoop =
          connect(this, &CScriptThread::SignalQuitLoop,
                  &loop, &QEventLoop::quit, Qt::QueuedConnection);
        QMetaObject::Connection interruptThisLoop =
          connect(this, &CScriptObjectBase::SignalInterruptExecution,
                  &loop, &QEventLoop::quit, Qt::QueuedConnection);

        // connect lambdas in loop context, so events are processed, but capture timer,
        // to start / stop
        QMetaObject::Connection pauseLoop =
          connect(this, &CScriptThread::SignalPauseTimer, &timer, &QTimer::stop,
                  Qt::QueuedConnection);
        QMetaObject::Connection resumeLoop =
          connect(this, &CScriptThread::SignalResumeTimer, &timer, [&timer, &lastTime]() {
            lastTime = QDateTime::currentDateTime();
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
            connect(spSignalEmitter.Get(), &CThreadSignalEmitter::waitSkipped,
                    &loop, &QEventLoop::quit, Qt::QueuedConnection);
        }

        timer.start();
        loop.exec();
        timer.stop();
        timer.disconnect();
        loop.disconnect();

        if (nullptr != pThis)
        {
          disconnect(quitLoop);
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
    }
  }
}

//----------------------------------------------------------------------------------------
//
QString CScriptThread::GetResourceName(const QVariant& resource, const QString& sMethod,
                                       bool bStringCanBeId, bool* pbOk)
{
  Q_UNUSED(bStringCanBeId)
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
    if (auto spComm = m_wpCommunicator.lock())
    {
      if (auto spSignalEmitter = spComm->LockedEmitter<CThreadSignalEmitter>())
      {
        emit spSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
      }
    }
    return QString();
  }
}
