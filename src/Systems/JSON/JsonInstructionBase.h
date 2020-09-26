#ifndef JSONINSTRUCTIONBASE_H
#define JSONINSTRUCTIONBASE_H

#include <QVariant>
#include <map>

class IJsonInstructionBase
{
public:
  IJsonInstructionBase() {}
  virtual ~IJsonInstructionBase() {}

  virtual const std::map<QString /*sName*/, QVariant::Type /*type*/>& ArgList() const = 0;
  virtual void Call(const QVariantMap& args) = 0;
};

#endif // JSONINSTRUCTIONBASE_H
