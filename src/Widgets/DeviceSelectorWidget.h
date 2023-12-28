#ifndef DEVICESELECTORWIDGET_H
#define DEVICESELECTORWIDGET_H

#include <QPointer>
#include <QWidget>

#include <memory>

class CDeviceManager;
class CDeviceViewDelegate;
class IDevice;
namespace Ui {
  class CDeviceSelectorWidget;
}
class QStandardItemModel;

class CDeviceSelectorWidget : public QWidget
{
  Q_OBJECT

public:
  explicit CDeviceSelectorWidget(QWidget* pParent = nullptr);
  ~CDeviceSelectorWidget();

signals:
  void SignalSelectedDevice(const QString& sName);
  void SignalSelectedDevice(const std::shared_ptr<IDevice>& sName);

protected slots:
  void on_pReconnectButton_clicked();
  void on_pStartScanButton_clicked();
  void on_pStopScanButton_clicked();
  void SlotActivatedItem(const QModelIndex& idx);
  void SlotConnected();
  void SlotDisconnected();
  void SlotDeviceCountChanged();
  void SlotStartScanning();
  void SlotStopScanning();

private:
  void ShowDevices();
  void SetConnectedState(bool bConnected);
  void SetScanningState(bool bScanning);

  std::unique_ptr<Ui::CDeviceSelectorWidget> m_spUi;
  std::weak_ptr<CDeviceManager>              m_wpDeviceManager;
  QPointer<QStandardItemModel>               m_pDeviceModel;
  QPointer<CDeviceViewDelegate>              m_pDeviceDelegate;
};

#endif // DEVICESELECTORWIDGET_H
