#include "CommandEosGotoBase.h"
#include "EosCommands.h"

CCommandEosGotoBase::CCommandEosGotoBase() :
  m_argTypes({
    { "target", SInstructionArgumentType{EArgumentType::eString}}
  })
{
}

CCommandEosGotoBase::~CCommandEosGotoBase()
{}

//----------------------------------------------------------------------------------------
//
tInstructionMapType& CCommandEosGotoBase::ArgList()
{
  return m_argTypes;
}

//----------------------------------------------------------------------------------------
//
IJsonInstructionBase::tRetVal CCommandEosGotoBase::Call(const tInstructionMapValue& args)
{
  Q_UNUSED(args)
  return SJsonException{"Not implemented call error.", "", eos::c_sCommandGoto, 0, 0};
}
