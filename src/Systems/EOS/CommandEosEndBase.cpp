#include "CommandEosEndBase.h"
#include "EosCommands.h"

CCommandEosEndBase::CCommandEosEndBase() :
  m_argTypes()
{}
CCommandEosEndBase::~CCommandEosEndBase()
{}

tInstructionMapType& CCommandEosEndBase::ArgList()
{
  return m_argTypes;
}

IJsonInstructionBase::tRetVal CCommandEosEndBase::Call(const tInstructionMapValue& args)
{
  Q_UNUSED(args)
  return SJsonException{"Not implemented call error.", "", eos::c_sCommandEnd, 0, 0};
}
