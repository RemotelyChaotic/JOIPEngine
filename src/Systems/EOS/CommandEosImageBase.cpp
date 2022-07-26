#include "CommandEosImageBase.h"
#include "EosCommands.h"

CCommandEosImageBase::CCommandEosImageBase() :
  m_argTypes({
    {"locator", SInstructionArgumentType{EArgumentType::eString}}
  })
{}
CCommandEosImageBase::~CCommandEosImageBase()
{
}

//----------------------------------------------------------------------------------------
//
tInstructionMapType& CCommandEosImageBase::ArgList()
{
  return m_argTypes;
}

//----------------------------------------------------------------------------------------
//
IJsonInstructionBase::tRetVal CCommandEosImageBase::Call(const tInstructionMapValue& args)
{
  Q_UNUSED(args)
  return SJsonException{"Not implemented call error.", "", eos::c_sCommandImage, 0, 0};
}
