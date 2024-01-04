#include "DeviceSelectorWidget.h"
#include "Application.h"
#include "ui_DeviceSelectorWidget.h"

#include "Systems/DeviceManager.h"

#include <QMovie>
#include <QPainter>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

namespace
{
  const qint32 c_iRoleDeviceName = Qt::UserRole+1;
  const qint32 c_iRoleScanningElement = Qt::UserRole+2;
  const qint32 c_iRoleSelectedElement = Qt::UserRole+3;
  const qint32 c_iLoadingIconSize = 16;
}

//----------------------------------------------------------------------------------------
//
class CDeviceViewDelegate : public QStyledItemDelegate
{
  Q_OBJECT
public:
  inline CDeviceViewDelegate(QPointer<QListView> pParent,
                             QPointer<CDeviceSelectorWidget> pDeviceSelectorWidget) :
    QStyledItemDelegate(pParent),
    m_pDeviceSelectorWidget(pDeviceSelectorWidget),
    m_pLoading(new QMovie(this))
  {
    m_pLoading.setFileName(":/resources/gif/spinner_transparent.gif");
    connect(&m_pLoading, &QMovie::frameChanged, [pParent]() {
      if (nullptr != pParent)
      {
        pParent->viewport()->update();
      }
    });
  }

  //--------------------------------------------------------------------------------------
  //
  void StartScanning()
  {
    m_pLoading.start();
  }

  //--------------------------------------------------------------------------------------
  //
  void StopScanning()
  {
    m_pLoading.stop();
  }

protected:
  //--------------------------------------------------------------------------------------
  //
  void paint(QPainter* pPainter, const QStyleOptionViewItem& option,
             const QModelIndex& index) const override
  {
    QStyledItemDelegate::paint(pPainter, option, index);
    if (index.data(c_iRoleScanningElement).toBool())
    {
      QSize iconSize = {c_iLoadingIconSize, c_iLoadingIconSize};
      QRect rectIconBounding = option.rect;
      rectIconBounding.setHeight(iconSize.height());
      qint32 iOffsetX = (rectIconBounding.width() - iconSize.width()) / 2;
      qint32 iOffsetY = (rectIconBounding.height() - iconSize.height()) / 2;
      QRect rectIcon({option.rect.x() + iOffsetX, option.rect.y() + iOffsetY}, iconSize);

      QImage img = m_pLoading.currentImage();
      pPainter->drawImage(rectIcon, img, img.rect());
    }
  }

  //--------------------------------------------------------------------------------------
  //
  QSize sizeHint(
      const QStyleOptionViewItem& options, const QModelIndex& index) const override
  {
    if (index.data(c_iRoleScanningElement).toBool())
    {
      return QSize(QStyledItemDelegate::sizeHint(options, index).width(), c_iLoadingIconSize);
    }
    return QStyledItemDelegate::sizeHint(options, index);
  }

private:
  QPointer<CDeviceSelectorWidget> m_pDeviceSelectorWidget;
  QMovie                          m_pLoading;
};

//----------------------------------------------------------------------------------------
//
CDeviceSelectorWidget::CDeviceSelectorWidget(QWidget* pParent) :
  QWidget(pParent),
  m_spUi(std::make_unique<Ui::CDeviceSelectorWidget>()),
  m_wpDeviceManager(CApplication::Instance()->System<CDeviceManager>()),
  m_selectionIcon(":/resources/style/img/ButtonPlug.png")
{
  m_spUi->setupUi(this);
  m_pDeviceModel = new QStandardItemModel(m_spUi->pDeviceListView);
  m_pDeviceDelegate = new CDeviceViewDelegate(m_spUi->pDeviceListView, this);
  m_spUi->pDeviceListView->setModel(m_pDeviceModel);
  m_spUi->pDeviceListView->setItemDelegate(m_pDeviceDelegate);

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
  const QString sName = m_pDeviceModel->data(idx, c_iRoleDeviceName).toString();
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
      const QString sName = m_pDeviceModel->data(idx, c_iRoleDeviceName).toString();
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
    QStringList vsDeviceNamesInUi;
    bool bHasScanningElement = false;
    for (qint32 i = 0; m_pDeviceModel->rowCount() > i; ++i)
    {
      QModelIndex idx = m_pDeviceModel->index(i, 0);
      const QString sName = m_pDeviceModel->data(idx, c_iRoleDeviceName).toString();
      bool bScanningElement =
          m_pDeviceModel->data(idx, c_iRoleScanningElement).toBool();
      if (!bScanningElement)
      {
        vsDeviceNamesInUi << sName;
      }
      else
      {
        bHasScanningElement = true;
      }
    }

    if (vsDeviceNames != vsDeviceNamesInUi)
    {
      m_pDeviceModel->clear();
      QList<QStandardItem*> vpItems;
      for (const QString& sName : qAsConst(vsDeviceNames))
      {
        QStandardItem* pItem = new QStandardItem(sName);
        pItem->setData(sName, c_iRoleDeviceName);
        pItem->setData(false, c_iRoleScanningElement);
        vpItems << pItem;
      }
      if (bHasScanningElement)
      {
        QStandardItem* pItem = new QStandardItem(QString());
        pItem->setData(QString(), c_iRoleDeviceName);
        pItem->setData(true, c_iRoleScanningElement);
        vpItems << pItem;
      }
      m_pDeviceModel->appendRow(vpItems);
    }
  }
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
    m_pDeviceDelegate->StartScanning();
    m_spUi->pStartScanButton->hide();
    m_spUi->pStopScanButton->show();
  }
  else
  {
    m_pDeviceDelegate->StopScanning();
    m_spUi->pStartScanButton->show();
    m_spUi->pStopScanButton->hide();
  }

  QModelIndex idx = m_pDeviceModel->index(m_pDeviceModel->rowCount()-1, 0);
  const bool bScanningElem = m_pDeviceModel->data(idx, c_iRoleScanningElement).toBool();
  if (bScanningElem && !bScanning)
  {
    m_pDeviceModel->removeRows(idx.row(), 1);
  }
  else if (!bScanningElem && bScanning)
  {
    QStandardItem* pItem = new QStandardItem(QString());
    pItem->setData(QString(), c_iRoleDeviceName);
    pItem->setData(true, c_iRoleScanningElement);
    m_pDeviceModel->appendRow(pItem);
  }
}

// private class CDeviceViewDelegate
#include "DeviceSelectorWidget.moc"
