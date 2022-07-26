#include "CommandEosEvalBase.h"
#include "EosCommands.h"

CCommandEosEvalBase::CCommandEosEvalBase() :
  m_argTypes({
    {"script", SInstructionArgumentType{EArgumentType::eString}}
  })
{}
CCommandEosEvalBase::~CCommandEosEvalBase()
{}

tInstructionMapType& CCommandEosEvalBase::ArgList()
{
  return m_argTypes;
}

IJsonInstructionBase::tRetVal CCommandEosEvalBase::Call(const tInstructionMapValue& args)
{
  Q_UNUSED(args)
  return SJsonException{"Not implemented call error.", "", eos::c_sCommandEval, 0, 0};
}
