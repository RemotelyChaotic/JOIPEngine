#ifndef CCOMMANDEOSIMAGEBASE_H
#define CCOMMANDEOSIMAGEBASE_H

#include "Systems/JSON/JsonInstructionBase.h"

class CCommandEosImageBase : public IJsonInstructionBase
{
public:
  CCommandEosImageBase();
  ~CCommandEosImageBase() override;

  tInstructionMapType& ArgList() override;
  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) override;
  tChildNodeGroups ChildNodeGroups(const tInstructionMapValue& args) const override;

private:
  tInstructionMapType    m_argTypes;
};

#endif // CCOMMANDEOSIMAGEBASE_H
