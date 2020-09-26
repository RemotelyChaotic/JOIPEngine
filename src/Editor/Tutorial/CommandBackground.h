#ifndef CTUTORIALCOMMANDBACKGROUND_H
#define CTUTORIALCOMMANDBACKGROUND_H

#include "Systems/JSON/JsonInstructionBase.h"

class CCommandBackground : public IJsonInstructionBase
{
public:
  CCommandBackground();
  ~CCommandBackground() override;

  const std::map<QString, QVariant::Type>& ArgList() const override;
  void Call(const QVariantMap& args) override;

private:
  const std::map<QString, QVariant::Type> m_argTypes;
};

#endif // CTUTORIALCOMMANDBACKGROUND_H
