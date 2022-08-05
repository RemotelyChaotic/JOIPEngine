#ifndef CCOMMANDEOSNOTIFICATIONCREATEBASE_H
#define CCOMMANDEOSNOTIFICATIONCREATEBASE_H

#include "Systems/JSON/JsonInstructionBase.h"

class CCommandEosNotificationCreateBase : public IJsonInstructionBase
{
public:
  CCommandEosNotificationCreateBase();
  ~CCommandEosNotificationCreateBase() override;

  tInstructionMapType& ArgList() override;
  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) override;
  tChildNodeGroups ChildNodeGroups(const tInstructionMapValue& args) const override;

private:
  tInstructionMapType    m_argTypes;
};

#endif // CCOMMANDEOSNOTIFICATIONCREATEBASE_H
