#ifndef CCOMMANDEOSTIMERBASE_H
#define CCOMMANDEOSTIMERBASE_H

#include "Systems/JSON/JsonInstructionBase.h"

class CCommandEosTimerBase : public IJsonInstructionBase
{
public:
  CCommandEosTimerBase();
  ~CCommandEosTimerBase() override;

  tInstructionMapType& ArgList() override;
  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) override;

private:
  tInstructionMapType    m_argTypes;
};

#endif // CCOMMANDEOSTIMERBASE_H
