#include "JsCodeGenerator.h"
#include "CodeGeneratorProvider.h"

DECLARE_CODE_GENERATOR(CJsCodeGenerator, SScriptDefinitionData::c_sScriptTypeJs)

//----------------------------------------------------------------------------------------
//
CJsCodeGenerator::CJsCodeGenerator() :
  CCommonCodeGenerator(SCommonCodeConfiguration{';', '[', ']', '(', ')', '\"', '.', '.', "//",
                                                "true", "false", "null", "let"})
{
}
CJsCodeGenerator::~CJsCodeGenerator()
{
}
