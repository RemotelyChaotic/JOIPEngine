#include "CommandText.h"

CCommandText::CCommandText() :
  IJsonInstructionBase(),
  m_argTypes({{"centerX", QVariant::Double}, {"centerY", QVariant::Double},
              {"text", QVariant::String}})
{

}
CCommandText::~CCommandText()
{

}

//----------------------------------------------------------------------------------------
//
const std::map<QString, QVariant::Type>& CCommandText::ArgList() const
{
  return m_argTypes;
}

//----------------------------------------------------------------------------------------
//
void CCommandText::Call(const QVariantMap& instruction)
{

}
