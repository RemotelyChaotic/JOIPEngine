#include "CommandEnd.h"

CCommandEnd::CCommandEnd() :
  IJsonInstructionBase(),
  m_argTypes()
{
}
CCommandEnd::~CCommandEnd()
{
}

//----------------------------------------------------------------------------------------
//
tInstructionMapType& CCommandEnd::ArgList()
{
  return m_argTypes;
}

//----------------------------------------------------------------------------------------
//
IJsonInstructionBase::tRetVal CCommandEnd::Call(const tInstructionMapValue& args)
{
  Q_UNUSED(args)
  return std::true_type();
}
