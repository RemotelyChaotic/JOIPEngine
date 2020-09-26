#include "CommandHighlight.h"

CCommandHighlight::CCommandHighlight() :
  IJsonInstructionBase(),
  m_argTypes({{"items", QVariant::StringList}})
{
}

CCommandHighlight::~CCommandHighlight()
{

}

//----------------------------------------------------------------------------------------
//
const std::map<QString, QVariant::Type>& CCommandHighlight::ArgList() const
{
  return m_argTypes;
}

//----------------------------------------------------------------------------------------
//
void CCommandHighlight::Call(const QVariantMap& instruction)
{

}
