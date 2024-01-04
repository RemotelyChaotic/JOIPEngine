#ifndef CBUTTPLUGDEVICE_H
#define CBUTTPLUGDEVICE_H

#include "IDevice.h"

#include <QMutex>
#include <QObject>
#include <QString>

#include <atomic>
#include <memory>

class CButtplugDeviceConnectorContext;
namespace Buttplug
{
  class Device;
}

class CButtplugDeviceWrapper : public IDevice
{
public:
  CButtplugDeviceWrapper(std::weak_ptr<Buttplug::Device> wpBpDevice,
                         CButtplugDeviceConnectorContext* pContext);
  ~CButtplugDeviceWrapper() override;

  QString Name() const override;
  void SendLinearCmd(quint32 iDurationMs, double dPosition) override;
  void SendRotateCmd(bool bClockwise, double dSpeed) override;
  void SendStopCmd() override;
  void SendVibrateCmd(double dSpeed) override;

private:
  std::shared_ptr<QMetaObject::Connection> m_spConnection;
  std::weak_ptr<Buttplug::Device> m_wpBpDevice;
  CButtplugDeviceConnectorContext* m_pContext;

  mutable QMutex                   m_nameMutex;
  QString                          m_sName;

  std::atomic<bool>                m_bDeviceValid = true;
};

#endif // CBUTTPLUGDEVICE_H
