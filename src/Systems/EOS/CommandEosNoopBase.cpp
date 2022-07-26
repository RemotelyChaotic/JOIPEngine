#include "CommandEosNoopBase.h"
#include "EosCommands.h"

CCommandEosNoopBase::CCommandEosNoopBase() :
  m_argTypes()
{}
CCommandEosNoopBase::~CCommandEosNoopBase()
{
}

//----------------------------------------------------------------------------------------
//
tInstructionMapType& CCommandEosNoopBase::ArgList()
{
  return m_argTypes;
}

//----------------------------------------------------------------------------------------
//
IJsonInstructionBase::tRetVal CCommandEosNoopBase::Call(const tInstructionMapValue& args)
{
  Q_UNUSED(args)
  return SJsonException{"Not implemented call error.", "", eos::c_sCommandNoop, 0, 0};
}
