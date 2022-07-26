#include "CommandEosNotificationCreateBase.h"
#include "EosCommands.h"

CCommandEosNotificationCreateBase::CCommandEosNotificationCreateBase() :
  m_argTypes({
    {"id", SInstructionArgumentType{EArgumentType::eString}},
    {"title", SInstructionArgumentType{EArgumentType::eString}},
    {"buttonLabel", SInstructionArgumentType{EArgumentType::eString}},
    {"timerDuration", SInstructionArgumentType{EArgumentType::eString}},
    {"onButtonCommand_Impl", SInstructionArgumentType{EArgumentType::eBool}},
    {"onTimerCommand_Impl", SInstructionArgumentType{EArgumentType::eBool}},
    {"timerDuration", SInstructionArgumentType{EArgumentType::eString}},
    {"buttonCommands", SInstructionArgumentType{EArgumentType::eArray,
             MakeArgArray(EArgumentType::eObject)}},
    {"timerCommands", SInstructionArgumentType{EArgumentType::eArray,
             MakeArgArray(EArgumentType::eObject)}},
  })
{
}
CCommandEosNotificationCreateBase::~CCommandEosNotificationCreateBase()
{
}

//----------------------------------------------------------------------------------------
//
tInstructionMapType& CCommandEosNotificationCreateBase::ArgList()
{
  return m_argTypes;
}

//----------------------------------------------------------------------------------------
//
IJsonInstructionBase::tRetVal CCommandEosNotificationCreateBase::Call(const tInstructionMapValue& args)
{
  Q_UNUSED(args)
  return SJsonException{"Not implemented call error.", "", eos::c_sCommandNotificationCreate, 0, 0};
}
