#include "CommandEosSayBase.h"
#include "EosCommands.h"

CCommandEosSayBase::CCommandEosSayBase() :
  m_argTypes({
    {"label", SInstructionArgumentType{EArgumentType::eString}},
    {"align", SInstructionArgumentType{EArgumentType::eString}},
    {"mode", SInstructionArgumentType{EArgumentType::eString}},
    {"allowSkip", SInstructionArgumentType{EArgumentType::eBool}},
    {"duration", SInstructionArgumentType{EArgumentType::eString}}
  })
{
}
CCommandEosSayBase::~CCommandEosSayBase()
{
}

//----------------------------------------------------------------------------------------
//
tInstructionMapType& CCommandEosSayBase::ArgList()
{
  return m_argTypes;
}

//----------------------------------------------------------------------------------------
//
IJsonInstructionBase::tRetVal CCommandEosSayBase::Call(const tInstructionMapValue& args)
{
  Q_UNUSED(args)
  return SJsonException{"Not implemented call error.", "", eos::c_sCommandSay, 0, 0};
}

//----------------------------------------------------------------------------------------
//
CCommandEosSayBase::tChildNodeGroups CCommandEosSayBase::ChildNodeGroups(const tInstructionMapValue&) const
{
  return {};
}
