#ifndef CDEVICEMANAGER_H
#define CDEVICEMANAGER_H

#include "ThreadedSystem.h"

#include <QMutex>
#include <QObject>

#include <memory>
#include <vector>

class IDevice;
class IDeviceConnector;

class CDeviceManager : public CSystemBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CDeviceManager)

public:
  CDeviceManager();
  ~CDeviceManager() override;

  void Connect();
  QStringList DeviceNames();
  std::shared_ptr<IDevice> Device(const QString& sName);
  std::vector<std::shared_ptr<IDevice>> Devices();
  void Disconnect();
  bool IsConnected() const;
  bool IsScanning() const;
  void StartScanning();
  void StopScanning();

signals:
  void SignalConnected();
  void SignalDisconnected();
  void SignalDeviceCountChanged();
  void SignalStartScanning();
  void SignalStopScanning();

public slots:
  void Initialize() override;
  void Deinitialize() override;

private slots:
  void ConnectImpl();
  QStringList DeviceNamesImpl();
  std::shared_ptr<IDevice> DeviceImpl(const QString& sName);
  std::vector<std::shared_ptr<IDevice>> DevicesImpl();
  void DisconnectImpl();
  bool IsConnectedImpl();
  bool IsScanningImpl();
  void SlotDisconnected();
  void StartScanningImpl();
  void StopScanningImpl();

private:
  IDeviceConnector*         m_pActiveConnector = nullptr;
  QMetaObject::Connection   m_disconnectConnection;
  QMetaObject::Connection   m_deviceCountChangedConnection;
  bool                      m_bIsScanning = false;
};

#endif // CDEVICEMANAGER_H
