#include "DeviceSelectorWidget.h"
#include "Application.h"
#include "ui_DeviceSelectorWidget.h"

#include "Systems/DeviceManager.h"
#include "Systems/Devices/IDevice.h"

#include <QMovie>
#include <QPainter>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

namespace
{
  const qint32 c_iRoleDeviceId = Qt::UserRole+1;
  const qint32 c_iRoleSelectedElement = Qt::UserRole+2;
  const qint32 c_iLoadingIconSize = 16;
}

//----------------------------------------------------------------------------------------
//
CDeviceSelectorWidget::CDeviceSelectorWidget(QWidget* pParent) :
  QWidget(pParent),
  m_spUi(std::make_unique<Ui::CDeviceSelectorWidget>()),
  m_wpDeviceManager(CApplication::Instance()->System<CDeviceManager>()),
  m_pLoadingLabel(new QLabel(this)),
  m_selectionIcon(":/resources/style/img/ButtonPlug.png")
{
  m_spUi->setupUi(this);
  m_pDeviceModel = new QStandardItemModel(m_spUi->pDeviceListView);
  m_spUi->pDeviceListView->setModel(m_pDeviceModel);

  m_loading.setFileName(":/resources/gif/spinner_transparent.gif");
  m_loading.setScaledSize({40, 40});
  m_pLoadingLabel->setMovie(&m_loading);
  m_pLoadingLabel->setScaledContents(true);
  m_pLoadingLabel->setFixedSize(40, 40);

  if (auto spDeviceManager = m_wpDeviceManager.lock())
  {
    SetConnectedState(spDeviceManager->IsConnected());
    SetScanningState(spDeviceManager->IsScanning());
    ShowDevices();
    SlotDeviceSelected();

    connect(spDeviceManager.get(), &CDeviceManager::SignalConnected, this,
            &CDeviceSelectorWidget::SlotConnected);
    connect(spDeviceManager.get(), &CDeviceManager::SignalDisconnected, this,
            &CDeviceSelectorWidget::SlotDisconnected);
    connect(spDeviceManager.get(), &CDeviceManager::SignalDeviceCountChanged, this,
            &CDeviceSelectorWidget::SlotDeviceCountChanged);
    connect(spDeviceManager.get(), &CDeviceManager::SignalDeviceSelected, this,
            &CDeviceSelectorWidget::SlotDeviceSelected);
    connect(spDeviceManager.get(), &CDeviceManager::SignalStartScanning, this,
            &CDeviceSelectorWidget::SlotStartScanning);
    connect(spDeviceManager.get(), &CDeviceManager::SignalStopScanning, this,
            &CDeviceSelectorWidget::SlotStopScanning);
  }

  connect(m_spUi->pDeviceListView, &QListView::activated,
          this, &CDeviceSelectorWidget::SlotActivatedItem);
}

CDeviceSelectorWidget::~CDeviceSelectorWidget()
{
}

//----------------------------------------------------------------------------------------
//
const QIcon& CDeviceSelectorWidget::SelectionIcon() const
{
  return m_selectionIcon;
}

//----------------------------------------------------------------------------------------
//
void CDeviceSelectorWidget::SetSelectionIcon(const QIcon& icon)
{
  m_selectionIcon = icon;
}

//----------------------------------------------------------------------------------------
//
void CDeviceSelectorWidget::on_pReconnectButton_clicked()
{
  if (auto spDeviceManager = m_wpDeviceManager.lock())
  {
    spDeviceManager->Connect();
  }
}

//----------------------------------------------------------------------------------------
//
void CDeviceSelectorWidget::on_pStartScanButton_clicked()
{
  if (auto spDeviceManager = m_wpDeviceManager.lock())
  {
    spDeviceManager->StartScanning();
  }
}

//----------------------------------------------------------------------------------------
//
void CDeviceSelectorWidget::on_pStopScanButton_clicked()
{
  if (auto spDeviceManager = m_wpDeviceManager.lock())
  {
    spDeviceManager->StopScanning();
  }
}

//----------------------------------------------------------------------------------------
//
void CDeviceSelectorWidget::SlotActivatedItem(const QModelIndex& idx)
{
  const QString sName = m_pDeviceModel->data(idx, c_iRoleDeviceId).toString();
  if (auto spDeviceManager = m_wpDeviceManager.lock())
  {
    spDeviceManager->SetSelectedDevice(sName);
    emit SignalSelectedDevice(sName);
  }
}

//----------------------------------------------------------------------------------------
//
void CDeviceSelectorWidget::SlotConnected()
{
  SetConnectedState(true);
}

//----------------------------------------------------------------------------------------
//
void CDeviceSelectorWidget::SlotDisconnected()
{
  SetConnectedState(false);
}

//----------------------------------------------------------------------------------------
//
void CDeviceSelectorWidget::SlotDeviceCountChanged()
{
  ShowDevices();
}

//----------------------------------------------------------------------------------------
//
void CDeviceSelectorWidget::SlotDeviceSelected()
{
  if (auto spDeviceManager = m_wpDeviceManager.lock())
  {
    const QString sDevice = spDeviceManager->SelectedDevice();
    for (qint32 i = 0; m_pDeviceModel->rowCount() > i; ++i)
    {
      QModelIndex idx = m_pDeviceModel->index(i, 0);
      const QString sName = m_pDeviceModel->data(idx, c_iRoleDeviceId).toString();
      if (sName == sDevice)
      {
        m_pDeviceModel->setData(idx, SelectionIcon(), Qt::DecorationRole);
        m_pDeviceModel->setData(idx, true, c_iRoleSelectedElement);
      }
      else
      {
        m_pDeviceModel->setData(idx, QVariant(), Qt::DecorationRole);
        m_pDeviceModel->setData(idx, false, c_iRoleSelectedElement);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CDeviceSelectorWidget::SlotStartScanning()
{
  SetScanningState(true);
}

//----------------------------------------------------------------------------------------
//
void CDeviceSelectorWidget::SlotStopScanning()
{
  SetScanningState(false);
}

//----------------------------------------------------------------------------------------
//
void CDeviceSelectorWidget::ShowDevices()
{
  if (auto spDeviceManager = m_wpDeviceManager.lock())
  {
    QStringList vsDeviceNames = spDeviceManager->DeviceNames();

    m_pDeviceModel->clear();
    QList<QStandardItem*> vpItems;
    for (const QString& sName : qAsConst(vsDeviceNames))
    {
      const QString sDisplayName = spDeviceManager->Device(sName)->DisplayName();
      QStandardItem* pItem = new QStandardItem(sDisplayName);
      pItem->setData(sName, c_iRoleDeviceId);
      vpItems << pItem;
    }
    m_pDeviceModel->appendRow(vpItems);
  }
}

//----------------------------------------------------------------------------------------
//
void CDeviceSelectorWidget::resizeEvent(QResizeEvent* pEvt)
{
  m_pLoadingLabel->move(width()/2-m_pLoadingLabel->width()/2,
                        height()- m_pLoadingLabel->height());
  m_pLoadingLabel->raise();
}

//----------------------------------------------------------------------------------------
//
void CDeviceSelectorWidget::SetConnectedState(bool bConnected)
{
  if (bConnected)
  {
    m_spUi->pStatusLabel->setText(tr("Connected"));
    m_spUi->pStartScanButton->setEnabled(true);
    m_spUi->pStopScanButton->setEnabled(true);
    m_spUi->pDeviceListView->setEnabled(true);
  }
  else
  {
    m_spUi->pStatusLabel->setText(tr("Disconnected"));
    m_spUi->pStartScanButton->setEnabled(false);
    m_spUi->pStopScanButton->setEnabled(false);
    m_spUi->pDeviceListView->setEnabled(false);
  }
}

//----------------------------------------------------------------------------------------
//
void CDeviceSelectorWidget::SetScanningState(bool bScanning)
{
  if (bScanning)
  {
    m_loading.start();
    m_pLoadingLabel->show();
    m_spUi->pStartScanButton->hide();
    m_spUi->pStopScanButton->show();
  }
  else
  {
    m_loading.stop();
    m_pLoadingLabel->hide();
    m_spUi->pStartScanButton->show();
    m_spUi->pStopScanButton->hide();
  }
}
