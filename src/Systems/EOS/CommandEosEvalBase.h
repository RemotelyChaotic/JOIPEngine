#ifndef CCOMMANDEOSEVALBASE_H
#define CCOMMANDEOSEVALBASE_H

#include "Systems/JSON/JsonInstructionBase.h"

class CCommandEosEvalBase : public IJsonInstructionBase
{
public:
  CCommandEosEvalBase();
  ~CCommandEosEvalBase() override;

  tInstructionMapType& ArgList() override;
  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) override;
  tChildNodeGroups ChildNodeGroups(const tInstructionMapValue& args) const override;

private:
  tInstructionMapType    m_argTypes;
};

#endif // CCOMMANDEOSEVALBASE_H
