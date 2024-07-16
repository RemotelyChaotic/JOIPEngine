#include "TeaseDeviceController.h"
#include "Application.h"
#include "SceneMainScreen.h"

#include "Systems/DeviceManager.h"
#include "Systems/Devices/IDevice.h"

#include "Widgets/DeviceSelectorWidget.h"

#include <QMenu>
#include <QWidgetAction>

CTeaseDeviceController::CTeaseDeviceController(QObject* pParent,
                                               QPointer<CSceneMainScreen> pMainScreen) :
  QObject{pParent},
  m_spCurrentDevice(nullptr),
  m_wpDeviceManager(CApplication::Instance()->System<CDeviceManager>()),
  m_pMainScreen(pMainScreen)
{
  if (auto spDeviceManager = m_wpDeviceManager.lock())
  {
    const QString sDevice = spDeviceManager->SelectedDevice();
    if (!sDevice.isEmpty())
    {
      m_spCurrentDevice = spDeviceManager->Device(sDevice);
    }
  }
}
CTeaseDeviceController::~CTeaseDeviceController() = default;

//----------------------------------------------------------------------------------------
//
qint32 CTeaseDeviceController::NumberRegisteredConnectors() const
{
  if (auto spDeviceManager = m_wpDeviceManager.lock())
  {
    return spDeviceManager->NumberRegisteredConnectors();
  }
  return 0;
}

//----------------------------------------------------------------------------------------
//
bool CTeaseDeviceController::isDeviceConnected()
{
  return nullptr != m_spCurrentDevice;
}

//----------------------------------------------------------------------------------------
//
void CTeaseDeviceController::openSelectionScreen(double x, double y)
{
  if (nullptr == m_pMainScreen) { return; }

  QPoint globalPoint = m_pMainScreen->mapToGlobal(QPoint(x,y));

  QMenu modelMenu;
  auto* pWidget = new CDeviceSelectorWidget(&modelMenu);

  QPointer<CTeaseDeviceController> pThis(this);
  connect(pWidget, &CDeviceSelectorWidget::SignalSelectedDevice, pWidget,
          [this, pThis](const QString& sName) {
    if (nullptr != pThis)
    {
      selectDevice(sName);
    }
  });

  auto* pAction = new QWidgetAction(&modelMenu);
  pAction->setDefaultWidget(pWidget);
  modelMenu.addAction(pAction);

  modelMenu.exec(globalPoint);
}

//----------------------------------------------------------------------------------------
//
void CTeaseDeviceController::pause()
{
  if (nullptr != m_spCurrentDevice)
  {
    m_spCurrentDevice->SendStopCmd();
  }
}

//----------------------------------------------------------------------------------------
//
void CTeaseDeviceController::resume()
{
  if (m_dlastRoateSpeed > 0)
  {
    sendRotateCmd(m_bLastRotateClockwise, m_dlastRoateSpeed);
  }
  if (m_ilastLinarDurationMs > 0)
  {
    sendLinearCmd(m_ilastLinarDurationMs, m_dlastLinearPosition);
  }
  if (m_dLastVibrateSpeed > 0)
  {
    sendVibrateCmd(m_dLastVibrateSpeed);
  }
}

//----------------------------------------------------------------------------------------
//
void CTeaseDeviceController::selectDevice(const QString& sDevice)
{
  if (auto spDeviceManager = m_wpDeviceManager.lock())
  {
    spDeviceManager->SetSelectedDevice(sDevice);
    if (!sDevice.isEmpty())
    {
      m_spCurrentDevice = spDeviceManager->Device(sDevice);
    }
    else
    {
      m_spCurrentDevice = nullptr;
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CTeaseDeviceController::sendLinearCmd(quint32 iDurationMs, double dPosition)
{
  if (nullptr != m_spCurrentDevice)
  {
    m_ilastLinarDurationMs = iDurationMs;
    m_dlastLinearPosition = dPosition;
    m_spCurrentDevice->SendLinearCmd(iDurationMs, dPosition);
  }
}

//----------------------------------------------------------------------------------------
//
void CTeaseDeviceController::sendRotateCmd(bool bClockwise, double dSpeed)
{
  if (nullptr != m_spCurrentDevice)
  {
    m_bLastRotateClockwise = bClockwise;
    m_dlastRoateSpeed = dSpeed;
    m_spCurrentDevice->SendRotateCmd(bClockwise, dSpeed);
  }
}

//----------------------------------------------------------------------------------------
//
void CTeaseDeviceController::sendStopCmd()
{
  if (nullptr != m_spCurrentDevice)
  {
    m_dLastVibrateSpeed = 0.0;
    m_ilastLinarDurationMs = 0;
    m_dlastLinearPosition = 0.0;
    m_bLastRotateClockwise = false;
    m_dlastRoateSpeed = 0.0;
    m_spCurrentDevice->SendStopCmd();
  }
}

//----------------------------------------------------------------------------------------
//
void CTeaseDeviceController::sendVibrateCmd(double dSpeed)
{
  if (nullptr != m_spCurrentDevice)
  {
    m_dLastVibrateSpeed = dSpeed;
    m_spCurrentDevice->SendVibrateCmd(dSpeed);
  }
}
