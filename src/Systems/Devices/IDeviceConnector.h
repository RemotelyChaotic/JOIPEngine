#ifndef IDEVICECONNECTOR_H
#define IDEVICECONNECTOR_H

#include "IDevice.h"

#include <QtPlugin>

#include <memory>
#include <vector>

class IDeviceConnector
{
public:
  virtual ~IDeviceConnector() {}

  virtual bool Connect() = 0;
  virtual void Disconnect() = 0;
  virtual QStringList DeviceNames() const = 0;
  virtual std::shared_ptr<IDevice> Device(const QString& sName) const = 0;
  virtual std::vector<std::shared_ptr<IDevice>> Devices() const = 0;
  virtual void StartScanning() = 0;
  virtual void StopScanning() = 0;

signals:
  virtual void SignalDeviceCountChanged() = 0;
  virtual void SignalDisconnected() = 0;

protected:
  IDeviceConnector() {}
};

Q_DECLARE_INTERFACE(IDeviceConnector, "IDeviceConnector")

#endif // IDEVICECONNECTOR_H
