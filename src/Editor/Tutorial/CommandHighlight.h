#ifndef CCOMMANDHIGHLIGHT_H
#define CCOMMANDHIGHLIGHT_H

#include "Systems/JSON/JsonInstructionBase.h"

class CCommandHighlight : public IJsonInstructionBase
{
public:
  CCommandHighlight();
  ~CCommandHighlight() override;

  const std::map<QString, QVariant::Type>& ArgList() const override;
  void Call(const QVariantMap& instruction) override;

private:
  const std::map<QString, QVariant::Type> m_argTypes;
};

#endif // CCOMMANDHIGHLIGHT_H
