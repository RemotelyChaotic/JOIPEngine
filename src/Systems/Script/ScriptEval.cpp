#include "ScriptEval.h"

#include "Systems/EOS/CommandEosEvalBase.h"
#include "Systems/EOS/CommandEosIfBase.h"
#include "Systems/EOS/EosCommands.h"

#include "Systems/JSON/JsonInstructionBase.h"
#include "Systems/JSON/JsonInstructionSetParser.h"

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
std::shared_ptr<CScriptObjectBase> CEvalSignalEmiter::CreateNewScriptObject(QPointer<QJSEngine> pEngine)
{
  return std::make_shared<CScriptEvalJs>(this, pEngine);
}
std::shared_ptr<CScriptObjectBase> CEvalSignalEmiter::CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser)
{
  return std::make_shared<CEosScriptEval>(this, pParser);
}
std::shared_ptr<CScriptObjectBase> CEvalSignalEmiter::CreateNewScriptObject(QtLua::State* pState)
{
  return std::make_shared<CScriptEvalLua>(this, pState);
}

//----------------------------------------------------------------------------------------
//
CScriptEvalBase::CScriptEvalBase(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                                 QPointer<QJSEngine> pEngine) :
  CJsScriptObjectBase(pEmitter, pEngine)
{}
CScriptEvalBase::CScriptEvalBase(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                                 QtLua::State* pState) :
  CJsScriptObjectBase(pEmitter, pState)
{}
CScriptEvalBase::~CScriptEvalBase()
{}

//----------------------------------------------------------------------------------------
//
QVariant CScriptEvalBase::EvalImpl(const QString& sScript)
{
  if (!CheckIfScriptCanRun()) { return QVariant(); }

  auto pSignalEmitter = SignalEmitter<CEvalSignalEmiter>();
  QTimer::singleShot(0, this, [&pSignalEmitter,sScript]() { emit pSignalEmitter->evalQuery(sScript); });

  // local loop to wait for answer
  QVariant returnValue = QVariant();
  QEventLoop loop;
  QMetaObject::Connection quitLoop =
    connect(this, &CScriptEvalBase::SignalQuitLoop, &loop, &QEventLoop::quit);
  QMetaObject::Connection interruptLoop =
    connect(pSignalEmitter, &CEvalSignalEmiter::interrupt,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
  QMetaObject::Connection interruptThisLoop =
    connect(this, &CScriptObjectBase::SignalInterruptExecution,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
  QMetaObject::Connection showRetValLoop =
    connect(pSignalEmitter, &CEvalSignalEmiter::evalReturn,
            this, [this, &returnValue](QJSValue input)
  {
    returnValue = input.toVariant();
    returnValue.detach(); // fixes some crashes with QJSEngine
    emit this->SignalQuitLoop();
    // direct connection to fix cross thread issues with QString content being deleted
  }, Qt::DirectConnection);
  loop.exec();
  loop.disconnect();

  disconnect(quitLoop);
  disconnect(interruptLoop);
  disconnect(interruptThisLoop);
  disconnect(showRetValLoop);

  return returnValue;
}

//----------------------------------------------------------------------------------------
//
CScriptEvalJs::CScriptEvalJs(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                             QPointer<QJSEngine> pEngine) :
  CScriptEvalBase(pEmitter, pEngine)
{}
CScriptEvalJs::~CScriptEvalJs() {}

//----------------------------------------------------------------------------------------
//
QJSValue CScriptEvalJs::eval(const QString& sScript)
{
  QVariant retVal = EvalImpl(sScript);
  return m_pEngine->toScriptValue(retVal);
}

//----------------------------------------------------------------------------------------
//
CScriptEvalLua::CScriptEvalLua(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                               QtLua::State* pState) :
  CScriptEvalBase(pEmitter, pState)
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
CEosScriptEval::CEosScriptEval(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                               QPointer<CJsonInstructionSetParser> pParser) :
  CEosScriptObjectBase(pEmitter, pParser),
  m_spCommandIf(std::make_shared<CCommandEosIf>(this)),
  m_spCommandEval(std::make_shared<CCommandEosEval>(this))
{
  pParser->RegisterInstruction(eos::c_sCommandIf, m_spCommandIf);
  pParser->RegisterInstruction(eos::c_sCommandEval, m_spCommandEval);
}
CEosScriptEval::~CEosScriptEval()
{}

//----------------------------------------------------------------------------------------
//
QVariant CEosScriptEval::eval(const QString& sScript)
{
  if (!CheckIfScriptCanRun()) { return QVariant(); }

  auto pSignalEmitter = SignalEmitter<CEvalSignalEmiter>();
  QTimer::singleShot(0, this, [pSignalEmitter,sScript]() {
    emit pSignalEmitter->evalQuery(sScript);
  });

  // local loop to wait for answer
  QVariant returnValue = QVariant();
  QEventLoop loop;
  QMetaObject::Connection quitLoop =
    connect(this, &CEosScriptEval::SignalQuitLoop, &loop, &QEventLoop::quit);
  QMetaObject::Connection interruptLoop =
    connect(pSignalEmitter, &CEvalSignalEmiter::interrupt,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
  QMetaObject::Connection interruptThisLoop =
    connect(this, &CScriptObjectBase::SignalInterruptExecution,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
  QMetaObject::Connection showRetValLoop =
    connect(pSignalEmitter, &CEvalSignalEmiter::evalReturn,
            this, [this, &returnValue](QJSValue input)
  {
    returnValue = input.toVariant();
    returnValue.detach(); // fixes some crashes with QJSEngine
    emit this->SignalQuitLoop();
    // direct connection to fix cross thread issues with QString content being deleted
  }, Qt::DirectConnection);
  loop.exec();
  loop.disconnect();

  disconnect(quitLoop);
  disconnect(interruptLoop);
  disconnect(interruptThisLoop);
  disconnect(showRetValLoop);

  return returnValue;
}
