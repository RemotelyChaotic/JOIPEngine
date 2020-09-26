#include "CommandBackground.h"

CCommandBackground::CCommandBackground() :
  IJsonInstructionBase(),
  m_argTypes({{"showBg", QVariant::Bool}, {"onCliCkAdvance", QVariant::Bool}})
{
}

CCommandBackground::~CCommandBackground()
{

}

//----------------------------------------------------------------------------------------
//
const std::map<QString, QVariant::Type>& CCommandBackground::ArgList() const
{
  return m_argTypes;
}

//----------------------------------------------------------------------------------------
//
void CCommandBackground::Call(const QVariantMap& args)
{

}
