#include "JsonInstructionSetParser.h"
#include "JsonInstructionSetRunner.h"
#include <nlohmann/json-schema.hpp>

CJsonInstructionSetParser::CJsonInstructionSetParser(QObject* pParent) :
  QObject(pParent)
{

}

CJsonInstructionSetParser::~CJsonInstructionSetParser()
{

}

//----------------------------------------------------------------------------------------
//
void CJsonInstructionSetParser::SetJsonSchema(const QByteArray& json)
{

}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CJsonInstructionSetRunner>
CJsonInstructionSetParser::ParseJson(const QByteArray& json)
{
  return std::make_shared<CJsonInstructionSetRunner>();
}
