#ifndef ICUSTOMMESSAGEHANDLER_H
#define ICUSTOMMESSAGEHANDLER_H

#include <QDebug>

class ICustomMessageHandler
{
public:
  virtual bool MessageImpl(QtMsgType type, const QMessageLogContext& context,
                           const QString& sMsg) = 0;

protected:
  ICustomMessageHandler() {}
  virtual ~ICustomMessageHandler() {};
};

#endif // ICUSTOMMESSAGEHANDLER_H
