#include "CommandEosIfBase.h"
#include "EosCommands.h"

CCommandEosIfBase::CCommandEosIfBase() :
  m_argTypes({
    {"condition", SInstructionArgumentType{EArgumentType::eString}},
    {"commands", SInstructionArgumentType{EArgumentType::eArray,
             MakeArgArray(EArgumentType::eObject)}},
    {"elseCommands", SInstructionArgumentType{EArgumentType::eArray,
             MakeArgArray(EArgumentType::eObject)}}
  })
{
}

CCommandEosIfBase::~CCommandEosIfBase()
{}

//----------------------------------------------------------------------------------------
//
tInstructionMapType& CCommandEosIfBase::ArgList()
{
  return m_argTypes;
}

//----------------------------------------------------------------------------------------
//
IJsonInstructionBase::tRetVal CCommandEosIfBase::Call(const tInstructionMapValue& args)
{
  Q_UNUSED(args)
  return SJsonException{"Not implemented call error.", "", eos::c_sCommandIf, 0, 0};
}

//----------------------------------------------------------------------------------------
//
CCommandEosIfBase::tChildNodeGroups CCommandEosIfBase::ChildNodeGroups(const tInstructionMapValue& args) const
{
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

  return {{QString("If"), QString("commands"), iCommandsIf},
          {QString("Else"), QString("elseCommands"), iCommandsElse}};
}
