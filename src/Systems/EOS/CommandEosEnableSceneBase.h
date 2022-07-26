#ifndef CCOMMANDEOSENABLESCENEBASE_H
#define CCOMMANDEOSENABLESCENEBASE_H

#include "Systems/JSON/JsonInstructionBase.h"

class CCommandEosEnableSceneBase : public IJsonInstructionBase
{
public:
  CCommandEosEnableSceneBase();
  ~CCommandEosEnableSceneBase() override;

  tInstructionMapType& ArgList() override;
  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) override;

private:
  tInstructionMapType    m_argTypes;
};

#endif // CCOMMANDEOSENABLESCENEBASE_H
