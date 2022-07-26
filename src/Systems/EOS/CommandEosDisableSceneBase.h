#ifndef CCOMMANDEOSDISABLESCENEBASE_H
#define CCOMMANDEOSDISABLESCENEBASE_H

#include "Systems/JSON/JsonInstructionBase.h"

class CCommandEosDisableSceneBase : public IJsonInstructionBase
{
public:
  CCommandEosDisableSceneBase();
  ~CCommandEosDisableSceneBase() override;

  tInstructionMapType& ArgList() override;
  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) override;

private:
  tInstructionMapType    m_argTypes;
};

#endif // CCOMMANDEOSDISABLESCENEBASE_H
