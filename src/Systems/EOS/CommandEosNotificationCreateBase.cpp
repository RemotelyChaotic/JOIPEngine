#include "CommandEosNotificationCreateBase.h"
#include "EosCommands.h"

CCommandEosNotificationCreateBase::CCommandEosNotificationCreateBase() :
  m_argTypes({
    {"id", SInstructionArgumentType{EArgumentType::eString}},
    {"title", SInstructionArgumentType{EArgumentType::eString}},
    {"buttonLabel", SInstructionArgumentType{EArgumentType::eString}},
    {"timerDuration", SInstructionArgumentType{EArgumentType::eString}},
    {"onButtonCommand_Impl", SInstructionArgumentType{EArgumentType::eBool}},
    {"onTimerCommand_Impl", SInstructionArgumentType{EArgumentType::eBool}},
    {"buttonCommands", SInstructionArgumentType{EArgumentType::eArray,
             MakeArgArray(EArgumentType::eObject)}},
    {"timerCommands", SInstructionArgumentType{EArgumentType::eArray,
             MakeArgArray(EArgumentType::eObject)}},
  })
{
}
CCommandEosNotificationCreateBase::~CCommandEosNotificationCreateBase()
{
}

//----------------------------------------------------------------------------------------
//
tInstructionMapType& CCommandEosNotificationCreateBase::ArgList()
{
  return m_argTypes;
}

//----------------------------------------------------------------------------------------
//
IJsonInstructionBase::tRetVal CCommandEosNotificationCreateBase::Call(const tInstructionMapValue& args)
{
  Q_UNUSED(args)
  return SJsonException{"Not implemented call error.", "", eos::c_sCommandNotificationCreate, 0, 0};
}

//----------------------------------------------------------------------------------------
//
CCommandEosNotificationCreateBase::tChildNodeGroups CCommandEosNotificationCreateBase::ChildNodeGroups(const tInstructionMapValue& args) const
{
  qint32 iCommandsButton = 0;
  qint32 iCommandsTimer = 0;
  const auto& itButtonCommands = GetValue<EArgumentType::eArray>(args, "buttonCommands");
  if (HasValue(args, "buttonCommands") && IsOk<EArgumentType::eArray>(itButtonCommands))
  {
    const tInstructionArrayValue& arrOptions = std::get<tInstructionArrayValue>(itButtonCommands);
    iCommandsButton = static_cast<quint32>(arrOptions.size());
  }
  const auto& itTimerCommands = GetValue<EArgumentType::eArray>(args, "timerCommands");
  if (HasValue(args, "timerCommands") && IsOk<EArgumentType::eArray>(itTimerCommands))
  {
    const tInstructionArrayValue& arrOptions = std::get<tInstructionArrayValue>(itTimerCommands);
    iCommandsTimer = static_cast<quint32>(arrOptions.size());
  }
  return {{QString("On Click"), iCommandsButton}, {QString("On Timeout"), iCommandsTimer}};
}
