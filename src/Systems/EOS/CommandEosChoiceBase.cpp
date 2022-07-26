#include "CommandEosChoiceBase.h"
#include "EosCommands.h"

CCommandEosChoiceBase::CCommandEosChoiceBase() :
  m_argTypes({
    {"options", SInstructionArgumentType{EArgumentType::eArray,
             MakeArgArray(EArgumentType::eMap, tInstructionMapType{
                   {"label", SInstructionArgumentType{EArgumentType::eString}},
                   {"commands", SInstructionArgumentType{EArgumentType::eArray,
                          MakeArgArray(EArgumentType::eObject)}},
                   {"color", SInstructionArgumentType{EArgumentType::eString}},
                   {"visible", SInstructionArgumentType{EArgumentType::eBool}}
             })
    }},
  })
{}
CCommandEosChoiceBase::~CCommandEosChoiceBase()
{}

//----------------------------------------------------------------------------------------
//
tInstructionMapType& CCommandEosChoiceBase::ArgList()
{
  return m_argTypes;
}

//----------------------------------------------------------------------------------------
//
IJsonInstructionBase::tRetVal CCommandEosChoiceBase::Call(const tInstructionMapValue& args)
{
  Q_UNUSED(args)
  return SJsonException{"Not implemented call error.", "", eos::c_sCommandChoice, 0, 0};
}
