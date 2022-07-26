#include "CommandEosPromptBase.h"
#include "EosCommands.h"

CCommandEosPromptBase::CCommandEosPromptBase() :
  m_argTypes({
    {"variable", SInstructionArgumentType{EArgumentType::eString}}
  })
{
}
CCommandEosPromptBase::~CCommandEosPromptBase()
{
}

//----------------------------------------------------------------------------------------
//
tInstructionMapType& CCommandEosPromptBase::ArgList()
{
  return m_argTypes;
}

//----------------------------------------------------------------------------------------
//
IJsonInstructionBase::tRetVal CCommandEosPromptBase::Call(const tInstructionMapValue& args)
{
  Q_UNUSED(args)
  return SJsonException{"Not implemented call error.", "", eos::c_sCommandPrompt, 0, 0};
}
