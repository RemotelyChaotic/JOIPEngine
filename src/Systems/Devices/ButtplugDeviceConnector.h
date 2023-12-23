#ifndef CBUTTPLUGDEVICECONNECTOR_H
#define CBUTTPLUGDEVICECONNECTOR_H

#include "IDeviceConnector.h"

#include <QObject>
#include <QPointer>
#include <QProcess>

#include <memory>

class CButtplugDeviceConnectorContext;
class CIntifaceEngineClientWrapper;
class QProcess;

class CButtplugDeviceConnector : public QObject, public IDeviceConnector
{
  Q_OBJECT
  Q_DISABLE_COPY(CButtplugDeviceConnector)
  Q_INTERFACES(IDeviceConnector)

public:
  CButtplugDeviceConnector();
  ~CButtplugDeviceConnector() override;

  QStringList DeviceNames() const override;
  std::vector<std::shared_ptr<IDevice>> Devices() const override;
  void StartScanning() override;
  void StopScanning() override;

signals:
  void SignalDeviceCountChanged() override;
  void SignalDisconnected() override;

protected:
  QString FindPluginPath() const;

  std::unique_ptr<CIntifaceEngineClientWrapper> m_spClient;
  QPointer<CButtplugDeviceConnectorContext>     m_pContext;
};

//----------------------------------------------------------------------------------------
//
class CIntifaceEngineDeviceConnector : public CButtplugDeviceConnector
{
  Q_OBJECT
  Q_DISABLE_COPY(CIntifaceEngineDeviceConnector)

public:
  CIntifaceEngineDeviceConnector();
  ~CIntifaceEngineDeviceConnector() override;

  bool Connect() override;
  void Disconnect() override;

protected slots:
  void SlotProcessError(QProcess::ProcessError error);
  void SlotReadyReadStdOut();

private:
  bool StartEngine();

  QPointer<QProcess>   m_pIntifaceEngineProcess;
};

//----------------------------------------------------------------------------------------
//
class CIntifaceCentralDeviceConnector : public CButtplugDeviceConnector
{
  Q_OBJECT
  Q_DISABLE_COPY(CIntifaceCentralDeviceConnector)

public:
  CIntifaceCentralDeviceConnector();
  ~CIntifaceCentralDeviceConnector() override;

  bool Connect() override;
  void Disconnect() override;

protected slots:
  void SlotProcessError(QProcess::ProcessError error);

private:
  bool FindIntifaceProcess();
  bool StartIntiface();
};

#endif // CBUTTPLUGDEVICECONNECTOR_H
