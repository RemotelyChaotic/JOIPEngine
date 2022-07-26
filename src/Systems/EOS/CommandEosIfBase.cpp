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
