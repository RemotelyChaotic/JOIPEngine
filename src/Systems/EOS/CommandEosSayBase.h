#ifndef CCOMMANDEOSSAYBASE_H
#define CCOMMANDEOSSAYBASE_H

#include "Systems/JSON/JsonInstructionBase.h"

class CCommandEosSayBase : public IJsonInstructionBase
{
public:
  CCommandEosSayBase();
  ~CCommandEosSayBase() override;

  tInstructionMapType& ArgList() override;
  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) override;

private:
  tInstructionMapType    m_argTypes;
};

#endif // CCOMMANDEOSSAYBASE_H
