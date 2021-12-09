#include "ScriptTimer.h"

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
std::shared_ptr<CScriptObjectBase> CTimerSignalEmitter::CreateNewScriptObject(QPointer<QJSEngine> pEngine)
{
  return std::make_shared<CScriptTimer>(this, pEngine);
}
std::shared_ptr<CScriptObjectBase> CTimerSignalEmitter::CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser)
{
  return std::make_shared<CEosScriptTimer>(this, pParser);
}


//----------------------------------------------------------------------------------------
//
CScriptTimer::CScriptTimer(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                           QPointer<QJSEngine> pEngine) :
  CJsScriptObjectBase(pEmitter, pEngine)
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
void CScriptTimer::setTime(double dTimeS)
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CTimerSignalEmitter>()->setTime(dTimeS);
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
  QMetaObject::Connection timeoutLoop =
    connect(pSignalEmitter, &CTimerSignalEmitter::timerFinished,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
  QMetaObject::Connection quitLoop =
    connect(pSignalEmitter, &CTimerSignalEmitter::interrupt,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
  QMetaObject::Connection interruptThisLoop =
    connect(this, &CScriptObjectBase::SignalInterruptExecution,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
  emit pSignalEmitter->waitForTimer();
  loop.exec();
  loop.disconnect();
  disconnect(timeoutLoop);
  disconnect(interruptThisLoop);
  disconnect(quitLoop);
}

//----------------------------------------------------------------------------------------
//
class CCommandEosTimer : public IJsonInstructionBase
{
public:
  CCommandEosTimer(CEosScriptTimer* pParent) :
    m_pParent(pParent),
    m_argTypes({
      {"duration", SInstructionArgumentType{EArgumentType::eString}},
      {"isAsync", SInstructionArgumentType{EArgumentType::eBool}},
      {"isAsync_threadImpl", SInstructionArgumentType{EArgumentType::eBool}},
      {"style", SInstructionArgumentType{EArgumentType::eString}},
      {"commands", SInstructionArgumentType{EArgumentType::eArray,
               MakeArgArray(EArgumentType::eObject)}},
    }) {}
  ~CCommandEosTimer() override {}

  tInstructionMapType& ArgList() override
  {
    return m_argTypes;
  }

  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) override
  {
    const auto& itIsAsync = GetValue<EArgumentType::eBool>(args, "isAsync");
    if (HasValue(args, "isAsync") && IsOk<EArgumentType::eBool>(itIsAsync))
    {
      tInstructionMapValue argsCopy = args;
      argsCopy.erase(args.find("isAsync"));
      argsCopy.insert({"isAsync_threadImpl", SInstructionArgumentValue{EArgumentType::eBool, true}});
      return SRunRetVal<ENextCommandToCall::eForkThis>({{argsCopy, QUuid::createUuid().toString(), true}});
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
          iTimeMs = eos::ParseEosDuration(std::get<QString>(itDuration));
          m_pParent->setTime(static_cast<double>(iTimeMs) / 1000);
        }

        bool bIsMainClock = true;
        if (HasValue(args, "style") && IsOk<EArgumentType::eString>(itStyle))
        {
          const QString sStyle = std::get<QString>(itStyle);
          if ("normal" == sStyle)
          {
            bIsMainClock = true;
            m_pParent->setTimeVisible(true);
            m_pParent->show();
          }
          else if ("hidden" == sStyle)
          {
            bIsMainClock = true;
            m_pParent->setTimeVisible(true);
            m_pParent->hide();
          }
          else if ("secret" == sStyle)
          {
            bIsMainClock = true;
            m_pParent->setTimeVisible(false);
            m_pParent->show();
          }
        }

        bool bIsInThread = true;
        const auto& itInThread = GetValue<EArgumentType::eBool>(args, "isAsync_threadImpl");
        if (HasValue(args, "isAsync_threadImpl") && IsOk<EArgumentType::eBool>(itInThread))
        {
          bIsInThread = std::get<bool>(itInThread);
        }

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
      return SJsonException{"internal Error.", "", "timer", 0, 0};
    }
    return SJsonException{"Invalid timer call.", "", "timer", 0, 0};
  }

private:
  CEosScriptTimer*       m_pParent;
  tInstructionMapType    m_argTypes;
};

//----------------------------------------------------------------------------------------
//
CEosScriptTimer::CEosScriptTimer(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                                 QPointer<CJsonInstructionSetParser> pParser) :
  CEosScriptObjectBase(pEmitter, pParser),
  m_spCommandTimer(std::make_shared<CCommandEosTimer>(this))
{
  pParser->RegisterInstruction("timer", m_spCommandTimer);
}
CEosScriptTimer::~CEosScriptTimer()
{
}

//----------------------------------------------------------------------------------------
//
void CEosScriptTimer::hide()
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CTimerSignalEmitter>()->hideTimer();
}

//----------------------------------------------------------------------------------------
//
void CEosScriptTimer::setTime(double dTimeS)
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CTimerSignalEmitter>()->setTime(dTimeS);
}

//----------------------------------------------------------------------------------------
//
void CEosScriptTimer::setTimeVisible(bool bVisible)
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CTimerSignalEmitter>()->setTimeVisible(bVisible);
}

//----------------------------------------------------------------------------------------
//
void CEosScriptTimer::sleep(qint64 iTimeMs)
{
  if (!CheckIfScriptCanRun()) { return; }

  auto pSignalEmitter = SignalEmitter<CTimerSignalEmitter>();

  if (0 < iTimeMs)
  {
    QDateTime lastTime = QDateTime::currentDateTime();
    qint64 iTimeLeft = iTimeMs;

    QTimer timer;
    timer.setSingleShot(false);
    timer.setInterval(20);
    QEventLoop loop;
    QMetaObject::Connection interruptLoop =
      connect(pSignalEmitter, &CTimerSignalEmitter::interrupt,
              &loop, &QEventLoop::quit, Qt::QueuedConnection);
    QMetaObject::Connection interruptThisLoop =
      connect(this, &CScriptObjectBase::SignalInterruptExecution,
              &loop, &QEventLoop::quit, Qt::QueuedConnection);

    // connect lambdas in loop context, so events are processed, but capture timer,
    // to start / stop
    QMetaObject::Connection pauseLoop =
      connect(pSignalEmitter, &CTimerSignalEmitter::pauseExecution, &loop, [&timer]() {
        timer.stop();
      }, Qt::QueuedConnection);
    QMetaObject::Connection resumeLoop =
      connect(pSignalEmitter, &CTimerSignalEmitter::resumeExecution, &loop, [&timer]() {
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

    disconnect(interruptLoop);
    disconnect(interruptThisLoop);
    disconnect(pauseLoop);
    disconnect(resumeLoop);
    disconnect(timeoutLoop);
  }
}

//----------------------------------------------------------------------------------------
//
void CEosScriptTimer::show()
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CTimerSignalEmitter>()->showTimer();
}

//----------------------------------------------------------------------------------------
//
void CEosScriptTimer::start()
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CTimerSignalEmitter>()->startTimer();
}

//----------------------------------------------------------------------------------------
//
void CEosScriptTimer::waitForTimer()
{
  if (!CheckIfScriptCanRun()) { return; }

  auto pSignalEmitter = SignalEmitter<CTimerSignalEmitter>();
  QEventLoop loop;
  QMetaObject::Connection timeoutLoop =
    connect(pSignalEmitter, &CTimerSignalEmitter::timerFinished,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
  QMetaObject::Connection quitLoop =
    connect(pSignalEmitter, &CTimerSignalEmitter::interrupt,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
  QMetaObject::Connection interruptThisLoop =
    connect(this, &CScriptObjectBase::SignalInterruptExecution,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
  emit pSignalEmitter->waitForTimer();
  loop.exec();
  loop.disconnect();
  disconnect(timeoutLoop);
  disconnect(interruptThisLoop);
  disconnect(quitLoop);
}
