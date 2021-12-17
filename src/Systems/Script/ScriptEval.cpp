#include "ScriptEval.h"
#include "Systems/JSON/JsonInstructionBase.h"
#include "Systems/JSON/JsonInstructionSetParser.h"

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
  return std::make_shared<CScriptEval>(this, pEngine);
}
std::shared_ptr<CScriptObjectBase> CEvalSignalEmiter::CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser)
{
  return std::make_shared<CEosScriptEval>(this, pParser);
}

//----------------------------------------------------------------------------------------
//
CScriptEval::CScriptEval(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                         QPointer<QJSEngine> pEngine) :
  CJsScriptObjectBase(pEmitter, pEngine)
{}
CScriptEval::~CScriptEval()
{}

//----------------------------------------------------------------------------------------
//
QJSValue CScriptEval::eval(const QString& sScript)
{
  if (!CheckIfScriptCanRun()) { return QJSValue(); }

  auto pSignalEmitter = SignalEmitter<CEvalSignalEmiter>();
  QTimer::singleShot(0, this, [&pSignalEmitter,sScript]() { emit pSignalEmitter->evalQuery(sScript); });

  // local loop to wait for answer
  QVariant returnValue = QVariant();
  QEventLoop loop;
  QMetaObject::Connection quitLoop =
    connect(this, &CScriptEval::SignalQuitLoop, &loop, &QEventLoop::quit);
  QMetaObject::Connection interruptLoop =
    connect(pSignalEmitter, &CEvalSignalEmiter::interrupt,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
  QMetaObject::Connection interruptThisLoop =
    connect(this, &CScriptObjectBase::SignalInterruptExecution,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
  QMetaObject::Connection showRetValLoop =
    connect(pSignalEmitter, &CEvalSignalEmiter::evalReturn,
            this, [this, &returnValue](QVariant input)
  {
    returnValue = input;
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

  return m_pEngine->toScriptValue(returnValue);
}

//----------------------------------------------------------------------------------------
//
class CCommandEosIf : public IJsonInstructionBase
{
public:
  CCommandEosIf(CEosScriptEval* pParent) :
    m_pParent(pParent),
    m_argTypes({
      {"condition", SInstructionArgumentType{EArgumentType::eString}},
      {"commands", SInstructionArgumentType{EArgumentType::eArray,
               MakeArgArray(EArgumentType::eObject)}},
      {"elseCommands", SInstructionArgumentType{EArgumentType::eArray,
               MakeArgArray(EArgumentType::eObject)}}
    }) {}
  ~CCommandEosIf() override {}

  tInstructionMapType& ArgList() override
  {
    return m_argTypes;
  }

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
    return SJsonException{"internal Error.", "", "prompt", 0, 0};
  }

private:
  CEosScriptEval*        m_pParent;
  tInstructionMapType    m_argTypes;
};


//----------------------------------------------------------------------------------------
//
class CCommandEosEval : public IJsonInstructionBase
{
public:
  CCommandEosEval(CEosScriptEval* pParent) :
    m_pParent(pParent),
    m_argTypes({
      {"script", SInstructionArgumentType{EArgumentType::eString}}
    }) {}
  ~CCommandEosEval() override {}

  tInstructionMapType& ArgList() override
  {
    return m_argTypes;
  }

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
    return SJsonException{"internal Error.", "script", "eval", 0, 0};
  }

private:
  CEosScriptEval*        m_pParent;
  tInstructionMapType    m_argTypes;
};


//----------------------------------------------------------------------------------------
//
CEosScriptEval::CEosScriptEval(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                               QPointer<CJsonInstructionSetParser> pParser) :
  CEosScriptObjectBase(pEmitter, pParser),
  m_spCommandIf(std::make_shared<CCommandEosIf>(this)),
  m_spCommandEval(std::make_shared<CCommandEosEval>(this))
{
  pParser->RegisterInstruction("if", m_spCommandIf);
  pParser->RegisterInstruction("eval", m_spCommandEval);
}
CEosScriptEval::~CEosScriptEval()
{}

//----------------------------------------------------------------------------------------
//
QVariant CEosScriptEval::eval(const QString& sScript)
{
  if (!CheckIfScriptCanRun()) { return QVariant(); }

  auto pSignalEmitter = SignalEmitter<CEvalSignalEmiter>();
  QTimer::singleShot(0, this, [&pSignalEmitter,sScript]() { emit pSignalEmitter->evalQuery(sScript); });

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
            this, [this, &returnValue](QVariant input)
  {
    returnValue = input;
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
