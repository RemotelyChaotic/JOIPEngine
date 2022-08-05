#ifndef CCOMMANDEOSNOTIFICATIONCLOSEBASE_H
#define CCOMMANDEOSNOTIFICATIONCLOSEBASE_H

#include "Systems/JSON/JsonInstructionBase.h"

class CCommandEosNotificationCloseBase : public IJsonInstructionBase
{
public:
  CCommandEosNotificationCloseBase();
  ~CCommandEosNotificationCloseBase() override;

  tInstructionMapType& ArgList() override;
  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) override;
  tChildNodeGroups ChildNodeGroups(const tInstructionMapValue& args) const override;

private:
  tInstructionMapType    m_argTypes;
};

#endif // CCOMMANDEOSNOTIFICATIONCLOSEBASE_H
