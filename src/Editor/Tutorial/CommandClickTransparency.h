#ifndef CCOMMANDCLICKTRANSPARENCY_H
#define CCOMMANDCLICKTRANSPARENCY_H

#include "Systems/JSON/JsonInstructionBase.h"

class CCommandClickTransparency : public IJsonInstructionBase
{
public:
  CCommandClickTransparency();
  ~CCommandClickTransparency() override;

  const std::map<QString, QVariant::Type>& ArgList() const override;
  void Call(const QVariantMap& instruction) override;

private:
  const std::map<QString, QVariant::Type> m_argTypes;
};

#endif // CCOMMANDCLICKTRANSPARENCY_H
