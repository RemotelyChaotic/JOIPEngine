#include "ScriptTimer.h"

#include "Systems/EOS/CommandEosTimerBase.h"
#include "Systems/EOS/EosCommands.h"
#include "Systems/EOS/EosHelpers.h"

#include "Systems/JSON/JsonInstructionBase.h"
#include "Systems/JSON/JsonInstructionSetParser.h"

#include <QDateTime>
#include <QEventLoop>
#include <QTimer>
#include <QUuid>

CTimerSignalEmitter::CTimerSignalEmitter() :
  CScriptRunnerSignalEmiter()
{

}
CTimerSignalEmitter::~CTimerSignalEmitter()
{

}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CScriptCommunicator>
CTimerSignalEmitter::CreateCommunicatorImpl(std::shared_ptr<CScriptRunnerSignalEmiterAccessor> spAccessor)
{
  return std::make_shared<CTimerScriptCommunicator>(spAccessor);
}

//----------------------------------------------------------------------------------------
//
CTimerScriptCommunicator::CTimerScriptCommunicator(
  const std::weak_ptr<CScriptRunnerSignalEmiterAccessor>& spEmitter) :
  CScriptCommunicator(spEmitter)
{}
CTimerScriptCommunicator::~CTimerScriptCommunicator() = default;

//----------------------------------------------------------------------------------------
//
CScriptObjectBase* CTimerScriptCommunicator::CreateNewScriptObject(QPointer<QJSEngine> pEngine)
{
  return new CScriptTimer(weak_from_this(), pEngine);
}
CScriptObjectBase* CTimerScriptCommunicator::CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser)
{
  return new CEosScriptTimer(weak_from_this(), pParser);
}
CScriptObjectBase* CTimerScriptCommunicator::CreateNewScriptObject(QtLua::State* pState)
{
  return new CScriptTimer(weak_from_this(), pState);
}
CScriptObjectBase* CTimerScriptCommunicator::CreateNewSequenceObject()
{
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
CScriptTimer::CScriptTimer(std::weak_ptr<CScriptCommunicator> pCommunicator,
                           QPointer<QJSEngine> pEngine) :
  CJsScriptObjectBase(pCommunicator, pEngine)
{
  m_spStop = std::make_shared<std::function<void()>>([this]() {
    emit SignalQuitLoop();
  });
  if (auto spComm = pCommunicator.lock())
  {
    spComm->RegisterStopCallback(m_spStop);

    if (auto spSignalEmitter = spComm->LockedEmitter<CTimerSignalEmitter>())
    {
      connect(spSignalEmitter.Get(), &CTimerSignalEmitter::timerFinished, this,
              &CScriptTimer::HandleTimerFinished, Qt::QueuedConnection);
    }
  }
}
CScriptTimer::CScriptTimer(std::weak_ptr<CScriptCommunicator> pCommunicator,
                           QtLua::State* pState) :
  CJsScriptObjectBase(pCommunicator, pState)
{
  m_spStop = std::make_shared<std::function<void()>>([this]() {
    emit SignalQuitLoop();
  });
  if (auto spComm = pCommunicator.lock())
  {
    spComm->RegisterStopCallback(m_spStop);

    if (auto spSignalEmitter = spComm->LockedEmitter<CTimerSignalEmitter>())
    {
      connect(spSignalEmitter.Get(), &CTimerSignalEmitter::timerFinished, this,
              &CScriptTimer::HandleTimerFinished, Qt::QueuedConnection);
    }
  }
}

CScriptTimer::~CScriptTimer()
{
  if (auto spComm = m_wpCommunicator.lock())
  {
    spComm->RemoveStopCallback(m_spStop);
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptTimer::hide()
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CTimerSignalEmitter>())
    {
      emit spSignalEmitter->hideTimer();
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptTimer::setTime(double dTimeS)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CTimerSignalEmitter>())
    {
      emit spSignalEmitter->setTime(dTimeS);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptTimer::setTimeVisible(bool bVisible)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CTimerSignalEmitter>())
    {
      emit spSignalEmitter->setTimeVisible(bVisible);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptTimer::show()
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CTimerSignalEmitter>())
    {
      emit spSignalEmitter->showTimer();
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptTimer::start()
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CTimerSignalEmitter>())
    {
      emit spSignalEmitter->startTimer();
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptTimer::stop()
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CTimerSignalEmitter>())
    {
      emit spSignalEmitter->stopTimer();
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptTimer::waitForTimer()
{
  if (!CheckIfScriptCanRun()) { return; }

  QTimer::singleShot(0, this, [this]() {
    if (auto spComm = m_wpCommunicator.lock())
    {
      if (auto spSignalEmitter = spComm->LockedEmitter<CTimerSignalEmitter>())
      {
        emit spSignalEmitter->waitForTimer();
      }
    }
  });

  QPointer<CScriptTimer> pThis(this);
  QEventLoop loop;
  QMetaObject::Connection timeoutLoop;
  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CTimerSignalEmitter>())
    {
      timeoutLoop =
        connect(spSignalEmitter.Get(), &CTimerSignalEmitter::timerFinished,
                &loop, &QEventLoop::quit, Qt::QueuedConnection);
    }
  }
  QMetaObject::Connection quitLoop =
    connect(this, &CScriptTimer::SignalQuitLoop,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
  QMetaObject::Connection interruptThisLoop =
    connect(this, &CScriptObjectBase::SignalInterruptExecution,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);

  loop.exec();
  loop.disconnect();

  if (nullptr != pThis)
  {
    if (timeoutLoop) disconnect(timeoutLoop);
    disconnect(quitLoop);
    disconnect(interruptThisLoop);
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptTimer::registerTimerFinishedCallback(QVariant callback)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (callback.canConvert<QtLua::Value>())
  {
    QtLua::Value fn = callback.value<QtLua::Value>();
    if (fn.type() == QtLua::Value::TFunction)
    {
      m_callback = fn;
    }
  }
  else if (callback.canConvert<QJSValue>())
  {
    QJSValue fn = callback.value<QJSValue>();
    if (fn.isCallable())
    {
      m_callback = fn;
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptTimer::HandleTimerFinished()
{
  if (!CheckIfScriptCanRun()) { return; }

  QString sError;
  if (nullptr != m_pEngine)
  {
    if (std::holds_alternative<QJSValue>(m_callback))
    {
      if (!script::CallCallback(std::get<QJSValue>(m_callback), QJSValueList(),
                                &sError))
      {
        if (auto spComm = m_wpCommunicator.lock())
        {
          if (auto spSignalEmitter = spComm->LockedEmitter<CTimerSignalEmitter>())
          {
            emit spSignalEmitter->showError(sError, QtMsgType::QtCriticalMsg);
          }
        }
      }
    }
  }
  else if (nullptr != m_pState)
  {
    if (std::holds_alternative<QtLua::Value>(m_callback))
    {
      if (!script::CallCallback(std::get<QtLua::Value>(m_callback), QVariantList(),
                                &sError))
      {
        if (auto spComm = m_wpCommunicator.lock())
        {
          if (auto spSignalEmitter = spComm->LockedEmitter<CTimerSignalEmitter>())
          {
            emit spSignalEmitter->showError(sError, QtMsgType::QtCriticalMsg);
          }
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
class CCommandEosTimer : public CCommandEosTimerBase
{
public:
  CCommandEosTimer(CEosScriptTimer* pParent) :
    CCommandEosTimerBase(),
    m_pParent(pParent)
  {}
  ~CCommandEosTimer() override {}

  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) override
  {
    const auto& itIsAsync = GetValue<EArgumentType::eBool>(args, "isAsync");
    if (HasValue(args, "isAsync") && IsOk<EArgumentType::eBool>(itIsAsync))
    {
      tInstructionMapValue argsCopy = args;
      auto itAsynchCopy = argsCopy.find("isAsync");
      if (argsCopy.end() != itAsynchCopy)
      {
        argsCopy.erase(itAsynchCopy);
      }
      argsCopy.insert({"isAsync_threadImpl", SInstructionArgumentValue{EArgumentType::eBool, true}});
      return SRunRetVal<ENextCommandToCall::eForkThis>(
            {{argsCopy, QUuid::createUuid().toString(), true, 0, -1}});
    }
    else
    {
      if (nullptr != m_pParent)
      {
        const auto& itDuration = GetValue<EArgumentType::eString>(args, "duration");
        const auto& itStyle = GetValue<EArgumentType::eString>(args, "style");

        qint64 iTimeMs = 0;
        if (HasValue(args, "duration") && IsOk<EArgumentType::eString>(itDuration))
        {
          auto duration = eos::ParseEosDuration(std::get<QString>(itDuration));
          if (std::holds_alternative<QString>(duration))
          {
            QString sRetVal = m_pParent->getTimerValue(std::get<QString>(duration));
            duration = eos::ParseEosDuration(sRetVal);
            if (!std::holds_alternative<QString>(duration))
            {
              iTimeMs = std::get<qint64>(duration);
            }
          }
          else
          {
            iTimeMs = std::get<qint64>(duration);
          }
          m_pParent->setTime(static_cast<double>(iTimeMs) / 1000);
        }

        bool bHasDefinedStyle = false;
        bool bIsMainClock = true;
        if (HasValue(args, "style") && IsOk<EArgumentType::eString>(itStyle))
        {
          const QString sStyle = std::get<QString>(itStyle);
          if (eos::c_vsTimerStyleStrings[eos::eNormal] == sStyle)
          {
            bHasDefinedStyle = true;
            bIsMainClock = true;
            m_pParent->setTimeVisible(true);
            m_pParent->show();
          }
          else if (eos::c_vsTimerStyleStrings[eos::eHidden] == sStyle)
          {
            bHasDefinedStyle = true;
            bIsMainClock = true;
            m_pParent->setTimeVisible(true);
            m_pParent->hide();
          }
          else if (eos::c_vsTimerStyleStrings[eos::eSecret] == sStyle)
          {
            bHasDefinedStyle = true;
            bIsMainClock = true;
            m_pParent->setTimeVisible(false);
            m_pParent->show();
          }
        }
        if (!bHasDefinedStyle)
        {
          m_pParent->setTimeVisible(true);
          m_pParent->show();
        }

        bool bIsInThread = true;
        const auto& itInThread = GetValue<EArgumentType::eBool>(args, "isAsync_threadImpl");
        if (HasValue(args, "isAsync_threadImpl") && IsOk<EArgumentType::eBool>(itInThread))
        {
          bIsInThread = std::get<bool>(itInThread);
        }
        Q_UNUSED(bIsInThread)

        if (bIsMainClock)
        {
          m_pParent->start();
          m_pParent->waitForTimer();
        }
        else
        {
          m_pParent->sleep(static_cast<double>(iTimeMs) / 1000);
        }

        const auto& itCommands = GetValue<EArgumentType::eArray>(args, "commands");
        if (HasValue(args, "commands") && IsOk<EArgumentType::eArray>(itCommands))
        {
          QStringList vList;
          tInstructionArrayValue items = std::get<tInstructionArrayValue>(itCommands);
          for (size_t i = 0; items.size() > i; ++i)
          {
            const auto& itCommand = GetValue<EArgumentType::eObject>(items, i);
            if (IsOk<EArgumentType::eString>(itCommand))
            {
              vList << std::get<QString>(itCommand);
            }
          }
          if (vList.size() > 0)
          {
            return SRunRetVal<ENextCommandToCall::eChild>(0);
          }
        }
        return SRunRetVal<ENextCommandToCall::eSibling>();
      }
      return SJsonException{"internal Error.", "", eos::c_sCommandTimer, 0, 0};
    }
  }

private:
  CEosScriptTimer*       m_pParent;
};

//----------------------------------------------------------------------------------------
//
CEosScriptTimer::CEosScriptTimer(std::weak_ptr<CScriptCommunicator> pCommunicator,
                                 QPointer<CJsonInstructionSetParser> pParser) :
  CEosScriptObjectBase(pCommunicator, pParser),
  m_spCommandTimer(std::make_shared<CCommandEosTimer>(this))
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

  pParser->RegisterInstruction(eos::c_sCommandTimer, m_spCommandTimer);
}
CEosScriptTimer::~CEosScriptTimer()
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
QString CEosScriptTimer::getTimerValue(QString sValue)
{
  QVariant var = RequestValue(sValue);
  if (var.canConvert(QVariant::String))
  {
    return var.toString();
  }
  return sValue;
}

//----------------------------------------------------------------------------------------
//
void CEosScriptTimer::hide()
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CTimerSignalEmitter>())
    {
      emit spSignalEmitter->hideTimer();
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEosScriptTimer::setTime(double dTimeS)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CTimerSignalEmitter>())
    {
      emit spSignalEmitter->setTime(dTimeS);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEosScriptTimer::setTimeVisible(bool bVisible)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CTimerSignalEmitter>())
    {
      emit spSignalEmitter->setTimeVisible(bVisible);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEosScriptTimer::sleep(qint64 iTimeMs)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (0 < iTimeMs)
  {
    QDateTime lastTime = QDateTime::currentDateTime();
    qint64 iTimeLeft = iTimeMs;

    QPointer<CEosScriptTimer> pThis(this);
    QTimer timer;
    timer.setSingleShot(false);
    timer.setInterval(20);
    QEventLoop loop;

    QMetaObject::Connection quitLoop =
      connect(this, &CEosScriptTimer::SignalQuitLoop,
              &loop, &QEventLoop::quit, Qt::QueuedConnection);
    QMetaObject::Connection interruptThisLoop =
      connect(this, &CScriptObjectBase::SignalInterruptExecution,
              &loop, &QEventLoop::quit, Qt::QueuedConnection);

    // connect lambdas in loop context, so events are processed, but capture timer,
    // to start / stop
    QMetaObject::Connection pauseLoop =
      connect(this, &CEosScriptTimer::SignalPauseTimer, &timer, &QTimer::stop,
              Qt::QueuedConnection);
    QMetaObject::Connection resumeLoop =
      connect(this, &CEosScriptTimer::SignalResumeTimer, &timer, [&timer, &lastTime]() {
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
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEosScriptTimer::show()
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CTimerSignalEmitter>())
    {
      emit spSignalEmitter->showTimer();
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEosScriptTimer::start()
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CTimerSignalEmitter>())
    {
      emit spSignalEmitter->startTimer();
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEosScriptTimer::waitForTimer()
{
  if (!CheckIfScriptCanRun()) { return; }

  QTimer::singleShot(0, this, [this]() {
    if (auto spComm = m_wpCommunicator.lock())
    {
      if (auto spSignalEmitter = spComm->LockedEmitter<CTimerSignalEmitter>())
      {
        emit spSignalEmitter->waitForTimer();
      }
    }
  });

  QPointer<CEosScriptTimer> pThis(this);
  QEventLoop loop;
  QMetaObject::Connection timeoutLoop;
  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CTimerSignalEmitter>())
    {
      timeoutLoop =
        connect(spSignalEmitter.Get(), &CTimerSignalEmitter::timerFinished,
                &loop, &QEventLoop::quit, Qt::QueuedConnection);
    }
  }
  QMetaObject::Connection quitLoop =
    connect(this, &CEosScriptTimer::SignalQuitLoop,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
  QMetaObject::Connection interruptThisLoop =
    connect(this, &CScriptObjectBase::SignalInterruptExecution,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);

  loop.exec();
  loop.disconnect();

  if (nullptr != pThis)
  {
    if (timeoutLoop) disconnect(timeoutLoop);
    disconnect(interruptThisLoop);
    disconnect(quitLoop);
  }
}
