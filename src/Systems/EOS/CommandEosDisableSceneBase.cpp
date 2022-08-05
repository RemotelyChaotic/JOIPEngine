#include "CommandEosDisableSceneBase.h"
#include "EosCommands.h"

CCommandEosDisableSceneBase::CCommandEosDisableSceneBase() :
  m_argTypes({
    {"target", SInstructionArgumentType{EArgumentType::eString}}
  })
{}
CCommandEosDisableSceneBase::~CCommandEosDisableSceneBase()
{}

tInstructionMapType& CCommandEosDisableSceneBase::ArgList()
{
  return m_argTypes;
}

IJsonInstructionBase::tRetVal CCommandEosDisableSceneBase::Call(const tInstructionMapValue& args)
{
  Q_UNUSED(args)
  return SJsonException{"Not implemented call error.", "", eos::c_sCommandDisableScreen, 0, 0};
}

//----------------------------------------------------------------------------------------
//
CCommandEosDisableSceneBase::tChildNodeGroups CCommandEosDisableSceneBase::ChildNodeGroups(const tInstructionMapValue&) const
{
  return {};
}
