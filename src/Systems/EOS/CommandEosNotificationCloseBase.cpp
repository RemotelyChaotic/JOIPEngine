#include "CommandEosNotificationCloseBase.h"
#include "EosCommands.h"

CCommandEosNotificationCloseBase::CCommandEosNotificationCloseBase() :
  m_argTypes({
    {"id", SInstructionArgumentType{EArgumentType::eString}}
  })
{

}
CCommandEosNotificationCloseBase::~CCommandEosNotificationCloseBase()
{
}

//----------------------------------------------------------------------------------------
//
tInstructionMapType& CCommandEosNotificationCloseBase::ArgList()
{
  return m_argTypes;
}

//----------------------------------------------------------------------------------------
//
IJsonInstructionBase::tRetVal CCommandEosNotificationCloseBase::Call(const tInstructionMapValue& args)
{
  Q_UNUSED(args)
  return SJsonException{"Not implemented call error.", "", eos::c_sCommandNotificationClose, 0, 0};
}

//----------------------------------------------------------------------------------------
//
CCommandEosNotificationCloseBase::tChildNodeGroups CCommandEosNotificationCloseBase::ChildNodeGroups(const tInstructionMapValue&) const
{
  return {};
}

