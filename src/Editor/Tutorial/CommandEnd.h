#ifndef COMMANDEND_H
#define COMMANDEND_H

#include "Systems/JSON/JsonInstructionBase.h"
#include <QPointer>

class CCommandEnd : public IJsonInstructionBase
{
public:
  CCommandEnd();
  ~CCommandEnd() override;

  tInstructionMapType& ArgList() override;
  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& instruction) override;

private:
  tInstructionMapType                     m_argTypes;
};

#endif // COMMANDEND_H
