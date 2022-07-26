#ifndef CCOMMANDEOSIFBASE_H
#define CCOMMANDEOSIFBASE_H

#include "Systems/JSON/JsonInstructionBase.h"

class CCommandEosIfBase : public IJsonInstructionBase
{
public:
  CCommandEosIfBase();
  ~CCommandEosIfBase() override;

  tInstructionMapType& ArgList() override;
  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) override;

private:
  tInstructionMapType    m_argTypes;
};

#endif // CCOMMANDEOSIFBASE_H
