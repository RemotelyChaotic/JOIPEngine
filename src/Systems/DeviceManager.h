#ifndef CDEVICEMANAGER_H
#define CDEVICEMANAGER_H

#include "ThreadedSystem.h"

#include <QMutex>
#include <QObject>

class IDeviceConnector;

class CDeviceManager : public CSystemBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CDeviceManager)

public:
  CDeviceManager();
  ~CDeviceManager() override;

  void Connect();
  void Disconnect();
  bool IsConnected() const;
  void StartScanning();
  void StopScanning();

public slots:
  void Initialize() override;
  void Deinitialize() override;

private slots:
  void ConnectImpl();
  void DisconnectImpl();
  bool IsConnectedImpl();
  void SlotDisconnected();
  void StartScanningImpl();
  void StopScanningImpl();

private:
  IDeviceConnector*         m_pActiveConnector = nullptr;
  QMetaObject::Connection   m_disconnectConnection;
};

#endif // CDEVICEMANAGER_H
