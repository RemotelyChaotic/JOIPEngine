#include "ScriptEval.h"

#include "Systems/EOS/CommandEosEvalBase.h"
#include "Systems/EOS/CommandEosIfBase.h"
#include "Systems/EOS/EosCommands.h"

#include "Systems/JSON/JsonInstructionBase.h"
#include "Systems/JSON/JsonInstructionSetParser.h"

#include "Systems/Sequence/SequenceEvalRunner.h"

#include <QtLua/Value>
#include <QtLua/State>

#include <QEventLoop>
#include <QTimer>

//----------------------------------------------------------------------------------------
//
CEvalSignalEmiter::CEvalSignalEmiter()
{

}
CEvalSignalEmiter::~CEvalSignalEmiter() {}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CScriptCommunicator>
CEvalSignalEmiter::CreateCommunicatorImpl(std::shared_ptr<CScriptRunnerSignalEmiterAccessor> spAccessor)
{
  return std::make_shared<CEvalScriptCommunicator>(spAccessor);
}

//----------------------------------------------------------------------------------------
//
CEvalScriptCommunicator::CEvalScriptCommunicator(
  const std::weak_ptr<CScriptRunnerSignalEmiterAccessor>& spEmitter) :
  CScriptCommunicator(spEmitter)
{}
CEvalScriptCommunicator::~CEvalScriptCommunicator() = default;

//----------------------------------------------------------------------------------------
//
CScriptObjectBase* CEvalScriptCommunicator::CreateNewScriptObject(QPointer<QJSEngine> pEngine)
{
  return new CScriptEvalJs(weak_from_this(), pEngine);
}
CScriptObjectBase* CEvalScriptCommunicator::CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser)
{
  return new CEosScriptEval(weak_from_this(), pParser);
}
CScriptObjectBase* CEvalScriptCommunicator::CreateNewScriptObject(QtLua::State* pState)
{
  return new CScriptEvalLua(weak_from_this(), pState);
}
CScriptObjectBase* CEvalScriptCommunicator::CreateNewSequenceObject()
{
  return new CSequenceEvalRunner(weak_from_this());
}

//----------------------------------------------------------------------------------------
//
CScriptEvalBase::CScriptEvalBase(std::weak_ptr<CScriptCommunicator> pCommunicator,
                                 QPointer<QJSEngine> pEngine) :
  CJsScriptObjectBase(pCommunicator, pEngine)
{
  m_spStop = std::make_shared<std::function<void()>>([this]() {
    emit SignalQuitLoop();
  });
  if (auto spComm = pCommunicator.lock())
  {
    spComm->RegisterStopCallback(m_spStop);
  }
}
CScriptEvalBase::CScriptEvalBase(std::weak_ptr<CScriptCommunicator> pCommunicator,
                                 QtLua::State* pState) :
  CJsScriptObjectBase(pCommunicator, pState)
{
  m_spStop = std::make_shared<std::function<void()>>([this]() {
    emit SignalQuitLoop();
  });
  if (auto spComm = pCommunicator.lock())
  {
    spComm->RegisterStopCallback(m_spStop);
  }
}
CScriptEvalBase::~CScriptEvalBase()
{
  if (auto spComm = m_wpCommunicator.lock())
  {
    spComm->RemoveStopCallback(m_spStop);
  }
}

//----------------------------------------------------------------------------------------
//
QVariant CScriptEvalBase::EvalImpl(const QString& sScript)
{
  if (!CheckIfScriptCanRun()) { return QVariant(); }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CEvalSignalEmiter>())
    {
      QTimer::singleShot(0, this, [this,sScript]() {
        if (auto spComm = m_wpCommunicator.lock())
        {
          if (auto spSignalEmitter = spComm->LockedEmitter<CEvalSignalEmiter>())
          {
            emit spSignalEmitter->evalQuery(sScript);
          }
        }
      });

      // local loop to wait for answer
      QPointer<CScriptEvalBase> pThis(this);
      std::shared_ptr<QVariant> spReturnValue = std::make_shared<QVariant>();
      QEventLoop loop;
      QMetaObject::Connection quitLoop =
        connect(this, &CScriptEvalBase::SignalQuitLoop, &loop, &QEventLoop::quit,
                Qt::QueuedConnection);
      QMetaObject::Connection interruptThisLoop =
        connect(this, &CScriptObjectBase::SignalInterruptExecution,
                &loop, &QEventLoop::quit, Qt::QueuedConnection);
      QMetaObject::Connection showRetValLoop =
        connect(spSignalEmitter.Get(), &CEvalSignalEmiter::evalReturn,
                &loop, [spReturnValue, &loop](QJSValue input)
      {
        *spReturnValue = input.toVariant();
        spReturnValue->detach(); // fixes some crashes with QJSEngine
        bool bOk = QMetaObject::invokeMethod(&loop, "quit", Qt::QueuedConnection);
        assert(bOk); Q_UNUSED(bOk)
        // direct connection to fix cross thread issues with QString content being deleted
      }, Qt::DirectConnection);
      loop.exec();
      loop.disconnect();

      if (nullptr != pThis)
      {
        disconnect(quitLoop);
        disconnect(interruptThisLoop);
        disconnect(showRetValLoop);
      }

      return *spReturnValue;
    }
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
CScriptEvalJs::CScriptEvalJs(std::weak_ptr<CScriptCommunicator> pCommunicator,
                             QPointer<QJSEngine> pEngine) :
  CScriptEvalBase(pCommunicator, pEngine)
{}
CScriptEvalJs::~CScriptEvalJs() {}

//----------------------------------------------------------------------------------------
//
QJSValue CScriptEvalJs::eval(const QString& sScript)
{
  QPointer<CScriptEvalJs> pThis(this);
  QVariant retVal = EvalImpl(sScript);
  if (nullptr != pThis)
  {
    return m_pEngine->toScriptValue(retVal);
  }
  return QJSValue();
}

//----------------------------------------------------------------------------------------
//
CScriptEvalLua::CScriptEvalLua(std::weak_ptr<CScriptCommunicator> pCommunicator,
                               QtLua::State* pState) :
  CScriptEvalBase(pCommunicator, pState)
{}
CScriptEvalLua::~CScriptEvalLua() {}

//----------------------------------------------------------------------------------------
//
QVariant CScriptEvalLua::eval(const QString& sScript)
{
  return EvalImpl(sScript);
}

//----------------------------------------------------------------------------------------
//
class CCommandEosIf : public CCommandEosIfBase
{
public:
  CCommandEosIf(CEosScriptEval* pParent) :
    CCommandEosIfBase(),
    m_pParent(pParent)
  {}
  ~CCommandEosIf() override {}

  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) override
  {
    if (nullptr != m_pParent)
    {
      const auto& itCondition = GetValue<EArgumentType::eString>(args, "condition");
      if (HasValue(args, "condition") && IsOk<EArgumentType::eString>(itCondition))
      {
        const QString sCondition = std::get<QString>(itCondition);

        qint32 iCommandsIf = 0;
        qint32 iCommandsElse = 0;
        const auto& itCommands = GetValue<EArgumentType::eArray>(args, "commands");
        if (HasValue(args, "commands") && IsOk<EArgumentType::eArray>(itCommands))
        {
          const tInstructionArrayValue& arrOptions = std::get<tInstructionArrayValue>(itCommands);
          iCommandsIf = static_cast<quint32>(arrOptions.size());
        }
        const auto& itCommandsElse = GetValue<EArgumentType::eArray>(args, "elseCommands");
        if (HasValue(args, "elseCommands") && IsOk<EArgumentType::eArray>(itCommandsElse))
        {
          const tInstructionArrayValue& arrOptions = std::get<tInstructionArrayValue>(itCommandsElse);
          iCommandsElse = static_cast<quint32>(arrOptions.size());
        }

        QVariant var = m_pParent->eval(sCondition);
        if (var.toBool())
        {
          if (iCommandsIf > 0)
          {
            return SRunRetVal<ENextCommandToCall::eChild>(0, iCommandsIf);
          }
        }
        else
        {
          if (iCommandsElse > 0)
          {
            return SRunRetVal<ENextCommandToCall::eChild>(iCommandsIf);
          }
        }
        return SRunRetVal<ENextCommandToCall::eSibling>();
      }
    }
    return SJsonException{"internal Error.", "", eos::c_sCommandIf, 0, 0};
  }

private:
  CEosScriptEval*        m_pParent;
};


//----------------------------------------------------------------------------------------
//
class CCommandEosEval : public CCommandEosEvalBase
{
public:
  CCommandEosEval(CEosScriptEval* pParent) :
    CCommandEosEvalBase(),
    m_pParent(pParent)
  {}
  ~CCommandEosEval() override {}

  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) override
  {
    if (nullptr != m_pParent)
    {
      const auto& itScript = GetValue<EArgumentType::eString>(args, "script");
      if (HasValue(args, "script") && IsOk<EArgumentType::eString>(itScript))
      {
        const QString sScript = std::get<QString>(itScript);
        m_pParent->eval(sScript);

        return SRunRetVal<ENextCommandToCall::eSibling>();
      }
    }
    return SJsonException{"internal Error.", "script", eos::c_sCommandEval, 0, 0};
  }

private:
  CEosScriptEval*        m_pParent;
};


//----------------------------------------------------------------------------------------
//
CEosScriptEval::CEosScriptEval(std::weak_ptr<CScriptCommunicator> pCommunicator,
                               QPointer<CJsonInstructionSetParser> pParser) :
  CEosScriptObjectBase(pCommunicator, pParser),
  m_spCommandIf(std::make_shared<CCommandEosIf>(this)),
  m_spCommandEval(std::make_shared<CCommandEosEval>(this))
{
  m_spStop = std::make_shared<std::function<void()>>([this]() {
    emit SignalQuitLoop();
  });
  if (auto spComm = pCommunicator.lock())
  {
    spComm->RegisterStopCallback(m_spStop);
  }
  pParser->RegisterInstruction(eos::c_sCommandIf, m_spCommandIf);
  pParser->RegisterInstruction(eos::c_sCommandEval, m_spCommandEval);
}
CEosScriptEval::~CEosScriptEval()
{
  if (auto spComm = m_wpCommunicator.lock())
  {
    spComm->RemoveStopCallback(m_spStop);
  }
}

//----------------------------------------------------------------------------------------
//
QVariant CEosScriptEval::eval(const QString& sScript)
{
  if (!CheckIfScriptCanRun()) { return QVariant(); }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CEvalSignalEmiter>())
    {
      QTimer::singleShot(0, this, [this,sScript]() {
        if (auto spComm = m_wpCommunicator.lock())
        {
          if (auto spSignalEmitter = spComm->LockedEmitter<CEvalSignalEmiter>())
          {
            emit spSignalEmitter->evalQuery(sScript);
          }
        }
      });

      // local loop to wait for answer
      QPointer<CEosScriptEval> pThis(this);
      std::shared_ptr<QVariant> spReturnValue = std::make_shared<QVariant>();
      QEventLoop loop;
      QMetaObject::Connection quitLoop =
        connect(this, &CEosScriptEval::SignalQuitLoop, &loop, &QEventLoop::quit,
                Qt::QueuedConnection);
      QMetaObject::Connection interruptThisLoop =
        connect(this, &CScriptObjectBase::SignalInterruptExecution,
                &loop, &QEventLoop::quit, Qt::QueuedConnection);
      QMetaObject::Connection showRetValLoop =
        connect(spSignalEmitter.Get(), &CEvalSignalEmiter::evalReturn,
                &loop, [spReturnValue, &loop](QJSValue input)
                {
                  *spReturnValue = input.toVariant();
                  spReturnValue->detach(); // fixes some crashes with QJSEngine
                  bool bOk = QMetaObject::invokeMethod(&loop, "quit", Qt::QueuedConnection);
                  assert(bOk); Q_UNUSED(bOk)
                  // direct connection to fix cross thread issues with QString content being deleted
                }, Qt::DirectConnection);
      loop.exec();
      loop.disconnect();

      if (nullptr != pThis)
      {
        disconnect(quitLoop);
        disconnect(interruptThisLoop);
        disconnect(showRetValLoop);
      }

      return *spReturnValue;
    }
  }
  return QVariant();
}
