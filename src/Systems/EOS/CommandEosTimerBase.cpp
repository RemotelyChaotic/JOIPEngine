#include "CommandEosTimerBase.h"
#include "EosCommands.h"

CCommandEosTimerBase::CCommandEosTimerBase() :
  m_argTypes({
    {"duration", SInstructionArgumentType{EArgumentType::eString}},
    {"isAsync", SInstructionArgumentType{EArgumentType::eBool}},
    {"isAsync_threadImpl", SInstructionArgumentType{EArgumentType::eBool}},
    {"style", SInstructionArgumentType{EArgumentType::eString}},
    {"commands", SInstructionArgumentType{EArgumentType::eArray,
             MakeArgArray(EArgumentType::eObject)}},
  })
{
}
CCommandEosTimerBase::~CCommandEosTimerBase()
{
}

//----------------------------------------------------------------------------------------
//
tInstructionMapType& CCommandEosTimerBase::ArgList()
{
  return m_argTypes;
}

//----------------------------------------------------------------------------------------
//
IJsonInstructionBase::tRetVal CCommandEosTimerBase::Call(const tInstructionMapValue& args)
{
  Q_UNUSED(args)
  return SJsonException{"Not implemented call error.", "", eos::c_sCommandTimer, 0, 0};
}

//----------------------------------------------------------------------------------------
//
CCommandEosTimerBase::tChildNodeGroups CCommandEosTimerBase::ChildNodeGroups(const tInstructionMapValue& args) const
{
  qint32 iCommands = 0;
  const auto& itCommands = GetValue<EArgumentType::eArray>(args, "commands");
  if (HasValue(args, "commands") && IsOk<EArgumentType::eArray>(itCommands))
  {
    tInstructionArrayValue arrOptions = std::get<tInstructionArrayValue>(itCommands);
    iCommands = static_cast<quint32>(arrOptions.size());
  }
  return {{QString("On Timeout"), iCommands}};
}
