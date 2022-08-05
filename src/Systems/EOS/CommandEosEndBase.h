#ifndef CCOMMANDEOSENDBASE_H
#define CCOMMANDEOSENDBASE_H

#include "Systems/JSON/JsonInstructionBase.h"

class CCommandEosEndBase : public IJsonInstructionBase
{
public:
  CCommandEosEndBase();
  ~CCommandEosEndBase() override;

  tInstructionMapType& ArgList() override;
  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) override;
  tChildNodeGroups ChildNodeGroups(const tInstructionMapValue& args) const override;

private:
  tInstructionMapType    m_argTypes;
};

#endif // CCOMMANDEOSENDBASE_H
