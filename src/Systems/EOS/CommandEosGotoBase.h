#ifndef CCOMMANDEOSGOTOBASE_H
#define CCOMMANDEOSGOTOBASE_H

#include "Systems/JSON/JsonInstructionBase.h"

class CCommandEosGotoBase : public IJsonInstructionBase
{
public:
  CCommandEosGotoBase();
  ~CCommandEosGotoBase() override;

  tInstructionMapType& ArgList() override;
  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) override;
  tChildNodeGroups ChildNodeGroups(const tInstructionMapValue& args) const override;

private:
  tInstructionMapType    m_argTypes;
};

#endif // CCOMMANDEOSGOTOBASE_H
