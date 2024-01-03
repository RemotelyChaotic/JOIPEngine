#include "DeviceManager.h"

#include "Application.h"
#include "Settings.h"

#if defined(HAS_BUTTPLUG_CPP)
  #include "Devices/ButtplugDeviceConnector.h"
#endif
#include "Devices/IDeviceConnector.h"

#include <QDebug>

Q_DECLARE_METATYPE(std::shared_ptr<IDevice>)
Q_DECLARE_METATYPE(std::vector<std::shared_ptr<IDevice>>)

namespace
{
  template<class T>
  struct movable_il {
    mutable T t;
    operator T() const&& { return std::move(t); }
    movable_il(T&& in ): t(std::move(in)) {}
    template <typename U>
    movable_il(U&& in): t(std::forward<U>(in)) {}
  };

  template<class T, class A=std::allocator<T>>
  std::vector<T,A> vector_from_il( std::initializer_list< movable_il<T> > il ) {
    std::vector<T,A> r( std::make_move_iterator(il.begin()), std::make_move_iterator(il.end()) );
    return r;
  }

  const std::vector<std::unique_ptr<IDeviceConnector>>& GetConnectors()
  {
    static std::vector<std::unique_ptr<IDeviceConnector>> vspAvailableConnectors =
    vector_from_il<std::unique_ptr<IDeviceConnector>> ({
      // extend here for more connectors (further down means higher priority in lookup)
      // - Register Buttplug.io connectors
      #if defined(HAS_BUTTPLUG_CPP)
        std::make_unique<CIntifaceEngineDeviceConnector>(),
        std::make_unique<CIntifaceCentralDeviceConnector>()
      #endif
    });
    return vspAvailableConnectors;
  }

  constexpr qint32 c_iReconnectInterval = 5000;
}

//----------------------------------------------------------------------------------------
//
CDeviceManager::CDeviceManager() :
  CSystemBase()
{
  qRegisterMetaType<std::shared_ptr<IDevice>>();
  qRegisterMetaType<std::vector<std::shared_ptr<IDevice>>>();
}

CDeviceManager::~CDeviceManager()
{
}

//----------------------------------------------------------------------------------------
//
void CDeviceManager::Connect()
{
  bool bOk = QMetaObject::invokeMethod(this, "ConnectImpl", Qt::QueuedConnection);
  assert(bOk); Q_UNUSED(bOk)
}

//----------------------------------------------------------------------------------------
//
QStringList CDeviceManager::DeviceNames()
{
  QStringList vsRet;
  bool bOk = QMetaObject::invokeMethod(const_cast<CDeviceManager*>(this), "DeviceNamesImpl",
                                       Qt::BlockingQueuedConnection,
                                       Q_RETURN_ARG(QStringList, vsRet));
  assert(bOk); Q_UNUSED(bOk)
  return vsRet;
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<IDevice> CDeviceManager::Device(const QString& sName)
{
  std::shared_ptr<IDevice> spRet;
  bool bOk = QMetaObject::invokeMethod(const_cast<CDeviceManager*>(this), "DeviceImpl",
                                       Qt::BlockingQueuedConnection,
                                       Q_ARG(QString, sName),
                                       Q_RETURN_ARG(std::shared_ptr<IDevice>, spRet));
  assert(bOk); Q_UNUSED(bOk)
  return spRet;
}

//----------------------------------------------------------------------------------------
//
std::vector<std::shared_ptr<IDevice>> CDeviceManager::Devices()
{
  std::vector<std::shared_ptr<IDevice>> vspRet;
  bool bOk = QMetaObject::invokeMethod(const_cast<CDeviceManager*>(this), "DevicesImpl",
                                       Qt::BlockingQueuedConnection,
                                       Q_RETURN_ARG(std::vector<std::shared_ptr<IDevice>>, vspRet));
  assert(bOk); Q_UNUSED(bOk)
  return vspRet;
}

//----------------------------------------------------------------------------------------
//
void CDeviceManager::Disconnect()
{
  bool bOk = QMetaObject::invokeMethod(this, "DisconnectImpl", Qt::QueuedConnection);
  assert(bOk); Q_UNUSED(bOk)
}

//----------------------------------------------------------------------------------------
//
bool CDeviceManager::IsConnected() const
{
  bool bRet = false;
  bool bOk = QMetaObject::invokeMethod(const_cast<CDeviceManager*>(this), "IsConnectedImpl",
                                       Qt::BlockingQueuedConnection,
                                       Q_RETURN_ARG(bool, bRet));
  assert(bOk); Q_UNUSED(bOk)
  return bRet;
}

//----------------------------------------------------------------------------------------
//
bool CDeviceManager::IsScanning() const
{
  bool bRet = false;
  bool bOk = QMetaObject::invokeMethod(const_cast<CDeviceManager*>(this), "IsScanningImpl",
                                       Qt::BlockingQueuedConnection,
                                       Q_RETURN_ARG(bool, bRet));
  assert(bOk); Q_UNUSED(bOk)
  return bRet;
}

//----------------------------------------------------------------------------------------
//
qint32 CDeviceManager::NumberRegisteredConnectors() const
{
  qint32 iRet = false;
  bool bOk = QMetaObject::invokeMethod(const_cast<CDeviceManager*>(this), "NumberRegisteredConnectorsImpl",
                                       Qt::BlockingQueuedConnection,
                                       Q_RETURN_ARG(qint32, iRet));
  assert(bOk); Q_UNUSED(bOk)
  return iRet;
}

//----------------------------------------------------------------------------------------
//
void CDeviceManager::StartScanning()
{
  bool bOk = QMetaObject::invokeMethod(this, "StartScanningImpl", Qt::QueuedConnection);
  assert(bOk); Q_UNUSED(bOk)
}

//----------------------------------------------------------------------------------------
//
void CDeviceManager::StopScanning()
{
  bool bOk = QMetaObject::invokeMethod(this, "StopScanningImpl", Qt::QueuedConnection);
  assert(bOk); Q_UNUSED(bOk)
}

//----------------------------------------------------------------------------------------
//
void CDeviceManager::Initialize()
{
  auto spSettings = CApplication::Instance()->Settings();
  if (nullptr != spSettings && spSettings->ConnectToHWOnStartup())
  {
    Connect();
  }
  SetInitialized(true);
}

//----------------------------------------------------------------------------------------
//
void CDeviceManager::Deinitialize()
{
  DeinitImpl();
  SetInitialized(false);
}

//----------------------------------------------------------------------------------------
//
void CDeviceManager::ConnectImpl()
{
  if (nullptr != m_pActiveConnector)
  {
    DisconnectImpl();
  }

  if (nullptr != m_pActiveConnector)
  {
    if (m_pActiveConnector->Connect())
    {
      if (nullptr != m_pReconnectTimer)
      {
        m_pReconnectTimer->stop();
      }
      emit SignalConnected();
    }
    else if (nullptr != m_pReconnectTimer)
    {
      m_pReconnectTimer->start();
    }
  }
  else
  {
    const auto& vspConnectors = GetConnectors();
    for (auto it = vspConnectors.rbegin(); vspConnectors.rend() != it; ++it)
    {
      // need to connect to this signal before connecting, since a connection might list
      // already connected devices on connect success
      // ...connect...
      m_deviceCountChangedConnection =
          connect(dynamic_cast<QObject*>(it->get()), SIGNAL(SignalDeviceCountChanged()),
                  this, SIGNAL(SignalDeviceCountChanged()), Qt::DirectConnection);
      assert(m_deviceCountChangedConnection);

      bool bConnected = false;
      if ((*it)->CanConnect())
      {
        bConnected = true;
        m_pActiveConnector = it->get();
        m_disconnectConnection =
            connect(dynamic_cast<QObject*>(m_pActiveConnector), SIGNAL(SignalDisconnected()),
                    this, SLOT(SlotDisconnected()), Qt::QueuedConnection);
        assert(m_disconnectConnection);

        auto spSettings = CApplication::Instance()->Settings();
        if (nullptr != spSettings && spSettings->ConnectToHWOnStartup())
        {
          if (nullptr == m_pReconnectTimer)
          {
            m_pReconnectTimer = new QTimer();
            m_pReconnectTimer->setInterval(c_iReconnectInterval);
            connect(m_pReconnectTimer, &QTimer::timeout, this,
                    &CDeviceManager::SlotReconnectTimerTimeout);
          }
        }

        if ((*it)->Connect())
        {
          if (nullptr != m_pReconnectTimer)
          {
            m_pReconnectTimer->stop();
          }
          emit SignalConnected();
        }
        else if (nullptr != m_pReconnectTimer)
        {
          m_pReconnectTimer->start();
        }
        break;
      }

      if (!bConnected)
      {
        disconnect(m_deviceCountChangedConnection);
      }
    }
  }

  if (nullptr == m_pActiveConnector)
  {
    qWarning() << "Could not connect any device connector.";
  }
}

//----------------------------------------------------------------------------------------
//
QStringList CDeviceManager::DeviceNamesImpl()
{
  if (IsConnectedImpl())
  {
    return m_pActiveConnector->DeviceNames();
  }
  return QStringList();
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<IDevice> CDeviceManager::DeviceImpl(const QString& sName)
{
  if (IsConnectedImpl())
  {
    return m_pActiveConnector->Device(sName);
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
std::vector<std::shared_ptr<IDevice>> CDeviceManager::DevicesImpl()
{
  if (IsConnectedImpl())
  {
    return m_pActiveConnector->Devices();
  }
  return {};
}

//----------------------------------------------------------------------------------------
//
void CDeviceManager::DeinitImpl()
{
  if (nullptr != m_pReconnectTimer)
  {
    m_pReconnectTimer->stop();
    delete m_pReconnectTimer;
  }

  if (nullptr != m_pActiveConnector)
  {
    disconnect(m_disconnectConnection);
    disconnect(m_deviceCountChangedConnection);
    m_pActiveConnector->Disconnect();
    m_pActiveConnector = nullptr;

    if (m_bIsScanning)
    {
      emit SignalStopScanning();
      m_bIsScanning = false;
    }

    emit SignalDisconnected();
  }
}

//----------------------------------------------------------------------------------------
//
void CDeviceManager::DisconnectImpl()
{
  if (nullptr != m_pReconnectTimer)
  {
    m_pReconnectTimer->stop();
  }

  if (nullptr != m_pActiveConnector)
  {
    m_pActiveConnector->Disconnect();

    if (m_bIsScanning)
    {
      emit SignalStopScanning();
      m_bIsScanning = false;
    }

    emit SignalDisconnected();
  }
}

//----------------------------------------------------------------------------------------
//
bool CDeviceManager::IsConnectedImpl()
{
  return nullptr != m_pActiveConnector && m_pActiveConnector->IsConnected();
}

//----------------------------------------------------------------------------------------
//
bool CDeviceManager::IsScanningImpl()
{
  return m_bIsScanning;
}

//----------------------------------------------------------------------------------------
//
void CDeviceManager::SlotDisconnected()
{
  // do a manual disconnect to properly clear everything
  DisconnectImpl();
}

//----------------------------------------------------------------------------------------
//
qint32 CDeviceManager::NumberRegisteredConnectorsImpl()
{
  return static_cast<qint32>(GetConnectors().size());
}

//----------------------------------------------------------------------------------------
//
void CDeviceManager::StartScanningImpl()
{
  if (IsConnectedImpl())
  {
    m_bIsScanning = true;
    m_pActiveConnector->StartScanning();
    emit SignalStartScanning();
  }
}

//----------------------------------------------------------------------------------------
//
void CDeviceManager::StopScanningImpl()
{
  if (IsConnectedImpl())
  {
    m_pActiveConnector->StopScanning();
    m_bIsScanning = false;
    emit SignalStopScanning();
  }
}

//----------------------------------------------------------------------------------------
//
void CDeviceManager::SlotReconnectTimerTimeout()
{
  ConnectImpl();
}
