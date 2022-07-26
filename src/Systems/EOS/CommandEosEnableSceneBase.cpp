#include "CommandEosEnableSceneBase.h"
#include "EosCommands.h"

CCommandEosEnableSceneBase::CCommandEosEnableSceneBase() :
  m_argTypes({
    {"target", SInstructionArgumentType{EArgumentType::eString}}
  })
{}
CCommandEosEnableSceneBase::~CCommandEosEnableSceneBase()
{}

tInstructionMapType& CCommandEosEnableSceneBase::ArgList()
{
  return m_argTypes;
}

IJsonInstructionBase::tRetVal CCommandEosEnableSceneBase::Call(const tInstructionMapValue& args)
{
  Q_UNUSED(args)
  return SJsonException{"Not implemented call error.", "", eos::c_sCommandEnableScreen, 0, 0};
}
