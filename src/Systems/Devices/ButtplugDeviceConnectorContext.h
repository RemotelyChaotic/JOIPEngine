#ifndef CBUTTPLUGDEVICECONNECTORCONTEXT_H
#define CBUTTPLUGDEVICECONNECTORCONTEXT_H

#include <QObject>

class CButtplugDeviceConnectorContext : public QObject
{
  Q_OBJECT

public:
  explicit CButtplugDeviceConnectorContext(QObject* pParent = nullptr);
  ~CButtplugDeviceConnectorContext();

signals:
  void SignalDeviceRemoved(QString sName);
};

#endif // CBUTTPLUGDEVICECONNECTORCONTEXT_H
