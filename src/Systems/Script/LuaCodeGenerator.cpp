#include "LuaCodeGenerator.h"
#include "CodeGeneratorProvider.h"
#include "Systems/DatabaseInterface/ResourceData.h"

DECLARE_CODE_GENERATOR(CLuaCodeGenerator, SScriptDefinitionData::c_sScriptTypeLua)

//----------------------------------------------------------------------------------------
//
CLuaCodeGenerator::CLuaCodeGenerator() :
  CCommonCodeGenerator(SCommonCodeConfiguration{';', '{', '}', '(', ')', '\"', ':', '.', "--",
                                                "true", "false", "nil", "local"})
{
}
CLuaCodeGenerator::~CLuaCodeGenerator()
{
}
