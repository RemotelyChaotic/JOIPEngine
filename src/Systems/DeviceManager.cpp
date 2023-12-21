#include "DeviceManager.h"

#if defined(HAS_BUTTPLUG_CPP)
  #include "Devices/ButtplugDeviceConnector.h"
#endif
#include "Devices/IDeviceConnector.h"

#include <QDebug>

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
        std::make_unique<CInitfaceEngineDeviceConnector>(),
        std::make_unique<CInitfaceCentralDeviceConnector>()
      #endif
    });
    return vspAvailableConnectors;
  }
}

//----------------------------------------------------------------------------------------
//
CDeviceManager::CDeviceManager() :
  CSystemBase()
{
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
  Connect();
  SetInitialized(true);
}

//----------------------------------------------------------------------------------------
//
void CDeviceManager::Deinitialize()
{
  Disconnect();
  SetInitialized(false);
}

//----------------------------------------------------------------------------------------
//
void CDeviceManager::ConnectImpl()
{
  if (nullptr != m_pActiveConnector)
  {
    Disconnect();
  }

  const auto& vspConnectors = GetConnectors();
  for (auto it = vspConnectors.rbegin(); vspConnectors.rend() != it; ++it)
  {
    if ((*it)->Connect())
    {
      m_pActiveConnector = it->get();
      m_disconnectConnection =
          connect(dynamic_cast<QObject*>(m_pActiveConnector), SIGNAL(SignalDisconnected()),
                  this, SLOT(SlotDisconnected()), Qt::QueuedConnection);
      assert(m_disconnectConnection);
      break;
    }
  }

  if (nullptr == m_pActiveConnector)
  {
    qWarning() << "Could not connect any device connector.";
  }
}

//----------------------------------------------------------------------------------------
//
void CDeviceManager::DisconnectImpl()
{
  if (nullptr != m_pActiveConnector)
  {
    disconnect(m_disconnectConnection);
    m_pActiveConnector->Disconnect();
    m_pActiveConnector = nullptr;
  }
}

//----------------------------------------------------------------------------------------
//
bool CDeviceManager::IsConnectedImpl()
{
  return nullptr != m_pActiveConnector;
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
void CDeviceManager::StartScanningImpl()
{
  if (IsConnectedImpl())
  {
    m_pActiveConnector->StartScanning();
  }
}

//----------------------------------------------------------------------------------------
//
void CDeviceManager::StopScanningImpl()
{
  if (IsConnectedImpl())
  {
    m_pActiveConnector->StopScanning();
  }
}
