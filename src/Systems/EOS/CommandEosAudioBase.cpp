#include "CommandEosAudioBase.h"
#include "EosCommands.h"

CCommandEosAudioBase::CCommandEosAudioBase() :
  m_argTypes({
    {"locator", SInstructionArgumentType{EArgumentType::eString}},
    {"id", SInstructionArgumentType{EArgumentType::eString}},
    {"loops", SInstructionArgumentType{EArgumentType::eInt64}},
    {"volume", SInstructionArgumentType{EArgumentType::eDouble}}, // ignored, engine manages volume
    {"seek", SInstructionArgumentType{EArgumentType::eInt64}},
    {"startAt", SInstructionArgumentType{EArgumentType::eInt64}}
  })
{}
CCommandEosAudioBase::~CCommandEosAudioBase()
{}

//----------------------------------------------------------------------------------------
//
tInstructionMapType& CCommandEosAudioBase::ArgList()
{
  return m_argTypes;
}

//----------------------------------------------------------------------------------------
//
IJsonInstructionBase::tRetVal CCommandEosAudioBase::Call(const tInstructionMapValue& args)
{
  Q_UNUSED(args)
  return SJsonException{"Not implemented call error.", "", eos::c_sCommandAudioPlay, 0, 0};
}
