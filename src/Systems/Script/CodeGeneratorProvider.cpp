#include "CodeGeneratorProvider.h"

CCodeGeneratorProvider::CCodeGeneratorProvider()
{
}
CCodeGeneratorProvider::~CCodeGeneratorProvider()
{
}

//----------------------------------------------------------------------------------------
//
CCodeGeneratorProvider& CCodeGeneratorProvider::Instance()
{
  static CCodeGeneratorProvider instance;
  return instance;
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<ICodeGenerator> CCodeGeneratorProvider::Generator(const QString& sCodeType) const
{
  auto it = m_generators.find(sCodeType);
  if (m_generators.end() != it)
  {
    return it->second;
  }
  return nullptr;
}
