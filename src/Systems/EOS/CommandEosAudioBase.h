#ifndef CCOMMANDEOSAUDIOBASE_H
#define CCOMMANDEOSAUDIOBASE_H

#include "Systems/JSON/JsonInstructionBase.h"

class CCommandEosAudioBase : public IJsonInstructionBase
{
public:
  CCommandEosAudioBase();
  ~CCommandEosAudioBase() override;

  tInstructionMapType& ArgList() override;
  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) override;

private:
  tInstructionMapType    m_argTypes;
};

#endif // CCOMMANDEOSAUDIOBASE_H
