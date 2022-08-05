#ifndef CCOMMANDEOSPROMPTBASE_H
#define CCOMMANDEOSPROMPTBASE_H

#include "Systems/JSON/JsonInstructionBase.h"

class CCommandEosPromptBase : public IJsonInstructionBase
{
public:
  CCommandEosPromptBase();
  ~CCommandEosPromptBase() override;

  tInstructionMapType& ArgList() override;
  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) override;
  tChildNodeGroups ChildNodeGroups(const tInstructionMapValue& args) const override;

private:
  tInstructionMapType    m_argTypes;
};

#endif // CCOMMANDEOSPROMPTBASE_H
