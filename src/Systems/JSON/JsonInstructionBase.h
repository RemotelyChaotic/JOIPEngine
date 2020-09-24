#ifndef JSONINSTRUCTIONBASE_H
#define JSONINSTRUCTIONBASE_H

#include <QJsonObject>

class JsonInstructionBase
{
public:
  JsonInstructionBase() {}
  virtual ~JsonInstructionBase() {}

  virtual QJsonObject GetSchema() = 0;

  virtual void operator()(const QJsonObject& instruction) = 0;
};

#endif // JSONINSTRUCTIONBASE_H
