#ifndef CCOMMANDTEXT_H
#define CCOMMANDTEXT_H

#include "Systems/JSON/JsonInstructionBase.h"

class CCommandText : public IJsonInstructionBase
{
public:
  CCommandText();
  ~CCommandText() override;

  const std::map<QString, QVariant::Type>& ArgList() const override;
  void Call(const QVariantMap& instruction) override;

private:
  const std::map<QString, QVariant::Type> m_argTypes;
};

#endif // CCOMMANDTEXT_H
