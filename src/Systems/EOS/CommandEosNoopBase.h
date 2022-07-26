#ifndef CCOMMANDEOSNOOPBASE_H
#define CCOMMANDEOSNOOPBASE_H

#include "Systems/JSON/JsonInstructionBase.h"

class CCommandEosNoopBase : public IJsonInstructionBase
{
public:
  CCommandEosNoopBase();
  ~CCommandEosNoopBase() override;

  tInstructionMapType& ArgList() override;
  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) override;

private:
  tInstructionMapType    m_argTypes;
};

#endif // CCOMMANDEOSNOOPBASE_H
