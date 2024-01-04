#include "ButtplugDeviceWrapper.h"
#include "ButtplugDeviceConnectorContext.h"

#include "Utils/ThreadUtils.h"

#include <ButtplugDevice.h>

CButtplugDeviceWrapper::CButtplugDeviceWrapper(std::weak_ptr<Buttplug::Device> wpBpDevice,
                                               CButtplugDeviceConnectorContext* pContext) :
  IDevice(),
  m_wpBpDevice(wpBpDevice),
  m_pContext(pContext),
  m_nameMutex(QMutex::Recursive)
{
  if (auto spDevice = m_wpBpDevice.lock())
  {
    m_sName = QString::fromStdString(spDevice->Name());
  }

  m_spConnection =
      std::make_shared<QMetaObject::Connection>();
  std::weak_ptr<QMetaObject::Connection> wpConnection = m_spConnection;
  *m_spConnection = QObject::connect(pContext, &CButtplugDeviceConnectorContext::SignalDeviceRemoved,
                                   pContext, [this, wpConnection](QString sName) mutable {
    if (auto spConnection = wpConnection.lock();
        nullptr != spConnection && Name() == sName && m_bDeviceValid.load())
    {
      m_bDeviceValid.store(false);
      m_pContext->disconnect(*spConnection);
      spConnection.reset();
    }
  });
}

CButtplugDeviceWrapper::~CButtplugDeviceWrapper()
{
  if (nullptr != m_spConnection)
  {
    m_pContext->disconnect(*m_spConnection);
    m_spConnection.reset();
  }
}

//----------------------------------------------------------------------------------------
//
QString CButtplugDeviceWrapper::Name() const
{
  QMutexLocker locker(&m_nameMutex);
  return m_sName;
}

//----------------------------------------------------------------------------------------
//
void CButtplugDeviceWrapper::SendLinearCmd(quint32 iDurationMs, double dPosition)
{
  if (!m_bDeviceValid.load()) { return; }

  utils::RunInThread(m_pContext->thread(), [wpDevice = m_wpBpDevice, iDurationMs, dPosition]() {
    if (auto spDevice = wpDevice.lock())
    {
      spDevice->SendLinearCmd(iDurationMs, dPosition);
    }
  });
}

//----------------------------------------------------------------------------------------
//
void CButtplugDeviceWrapper::SendRotateCmd(bool bClockwise, double dSpeed)
{
  if (!m_bDeviceValid.load()) { return; }

  utils::RunInThread(m_pContext->thread(), [wpDevice = m_wpBpDevice, bClockwise, dSpeed]() {
    if (auto spDevice = wpDevice.lock())
    {
      spDevice->SendRotateCmd(bClockwise, dSpeed);
    }
  });
}

//----------------------------------------------------------------------------------------
//
void CButtplugDeviceWrapper::SendStopCmd()
{
  if (!m_bDeviceValid.load()) { return; }

  utils::RunInThread(m_pContext->thread(), [wpDevice = m_wpBpDevice]() {
    if (auto spDevice = wpDevice.lock())
    {
      spDevice->SetndStopCmd();
    }
  });
}

//----------------------------------------------------------------------------------------
//
void CButtplugDeviceWrapper::SendVibrateCmd(double dSpeed)
{
  if (!m_bDeviceValid.load()) { return; }

  utils::RunInThread(m_pContext->thread(), [wpDevice = m_wpBpDevice, dSpeed]() {
    if (auto spDevice = wpDevice.lock())
    {
      spDevice->SendVibrateCmd(dSpeed);
    }
  });
}
