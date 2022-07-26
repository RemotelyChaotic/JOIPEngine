#ifndef CCOMMANDEOSCHOICEBASE_H
#define CCOMMANDEOSCHOICEBASE_H

#include "Systems/JSON/JsonInstructionBase.h"

class CCommandEosChoiceBase : public IJsonInstructionBase
{
public:
  CCommandEosChoiceBase();
  ~CCommandEosChoiceBase() override;

  tInstructionMapType& ArgList() override;
  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) override;

private:
  tInstructionMapType    m_argTypes;
};

#endif // CCOMMANDEOSCHOICEBASE_H
