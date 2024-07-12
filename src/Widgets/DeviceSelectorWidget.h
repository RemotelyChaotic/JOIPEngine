#ifndef DEVICESELECTORWIDGET_H
#define DEVICESELECTORWIDGET_H

#include <QIcon>
#include <QLabel>
#include <QMovie>
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
  Q_PROPERTY(QIcon selectionIcon READ SelectionIcon WRITE SetSelectionIcon)

public:
  explicit CDeviceSelectorWidget(QWidget* pParent = nullptr);
  ~CDeviceSelectorWidget();

  const QIcon& SelectionIcon() const;
  void SetSelectionIcon(const QIcon& icon);

signals:
  void SignalSelectedDevice(const QString& sName);

protected slots:
  void on_pReconnectButton_clicked();
  void on_pStartScanButton_clicked();
  void on_pStopScanButton_clicked();
  void SlotActivatedItem(const QModelIndex& idx);
  void SlotConnected();
  void SlotDisconnected();
  void SlotDeviceCountChanged();
  void SlotDeviceSelected();
  void SlotStartScanning();
  void SlotStopScanning();

protected:
  void resizeEvent(QResizeEvent* pEvt) override;

private:
  void ShowDevices();
  void SetConnectedState(bool bConnected);
  void SetScanningState(bool bScanning);

  std::unique_ptr<Ui::CDeviceSelectorWidget> m_spUi;
  std::weak_ptr<CDeviceManager>              m_wpDeviceManager;
  QPointer<QStandardItemModel>               m_pDeviceModel;
  QPointer<CDeviceViewDelegate>              m_pDeviceDelegate;
  QPointer<QLabel>                           m_pLoadingLabel;
  QMovie                                     m_loading;
  QIcon                                      m_selectionIcon;
};

#endif // DEVICESELECTORWIDGET_H
