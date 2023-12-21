#ifndef IDEVICECONNECTOR_H
#define IDEVICECONNECTOR_H

#include <QtPlugin>

class IDeviceConnector
{
public:
  virtual ~IDeviceConnector() {}

  virtual bool Connect() = 0;
  virtual void Disconnect() = 0;
  virtual void StartScanning() = 0;
  virtual void StopScanning() = 0;

signals:
  virtual void SignalDisconnected() = 0;

protected:
  IDeviceConnector() {}
};

Q_DECLARE_INTERFACE(IDeviceConnector, "IDeviceConnector")

#endif // IDEVICECONNECTOR_H
