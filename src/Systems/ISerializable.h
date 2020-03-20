#ifndef ISERIALIZABLE_H
#define ISERIALIZABLE_H

#include <QJsonObject>

struct ISerializable
{
  virtual ~ISerializable() {}

  virtual QJsonObject ToJsonObject() = 0;
  virtual void FromJsonObject(const QJsonObject& json) = 0;

  operator QJsonObject()
  {
    return ToJsonObject();
  }

  ISerializable& operator=(const QJsonObject& json)
  {
    FromJsonObject(json);
    return *this;
  }
};

#endif // ISERIALIZABLE_H
