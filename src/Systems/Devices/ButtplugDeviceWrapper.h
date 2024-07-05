#ifndef CBUTTPLUGDEVICE_H
#define CBUTTPLUGDEVICE_H

#include "IDevice.h"

#include <QButtplugClientDevice>

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
  CButtplugDeviceWrapper(QButtplugClientDevice device,
                         CButtplugDeviceConnectorContext* pContext);
  ~CButtplugDeviceWrapper() override;

  QString Name() const override;
  QString DisplayName() const override;
  void SendLinearCmd(quint32 iDurationMs, double dPosition) override;
  void SendRotateCmd(bool bClockwise, double dSpeed) override;
  void SendStopCmd() override;
  void SendVibrateCmd(double dSpeed) override;

private:
  std::shared_ptr<QMetaObject::Connection> m_spConnection;
  QButtplugClientDevice            m_device;
  CButtplugDeviceConnectorContext* m_pContext;

  mutable QMutex                   m_nameMutex;
  QString                          m_sName;
  QString                          m_sDisplayName;

  std::atomic<bool>                m_bDeviceValid = true;
};

#endif // CBUTTPLUGDEVICE_H
