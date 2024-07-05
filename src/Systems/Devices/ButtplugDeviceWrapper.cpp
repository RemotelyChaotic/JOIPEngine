#include "ButtplugDeviceWrapper.h"
#include "ButtplugDeviceConnectorContext.h"

#include "Utils/ThreadUtils.h"

CButtplugDeviceWrapper::CButtplugDeviceWrapper(QButtplugClientDevice device,
                                               CButtplugDeviceConnectorContext* pContext) :
  IDevice(),
  m_device(device),
  m_pContext(pContext),
  m_nameMutex(QMutex::Recursive)
{
  if (m_device.isValid())
  {
    m_sName = QString::number(device.id());
    m_sDisplayName = !device.displayName().isEmpty() ? device.displayName() :
                                                       device.name();
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
QString CButtplugDeviceWrapper::DisplayName() const
{
  QMutexLocker locker(&m_nameMutex);
  return m_sDisplayName;
}

//----------------------------------------------------------------------------------------
//
void CButtplugDeviceWrapper::SendLinearCmd(quint32 iDurationMs, double dPosition)
{
  if (!m_bDeviceValid.load()) { return; }

  utils::RunInThread(m_pContext->thread(), [device = m_device, iDurationMs, dPosition]() mutable {
    if (device.isValid())
    {
      device.sendLinearCmd(iDurationMs, dPosition, -1);
    }
  });
}

//----------------------------------------------------------------------------------------
//
void CButtplugDeviceWrapper::SendRotateCmd(bool bClockwise, double dSpeed)
{
  if (!m_bDeviceValid.load()) { return; }

  utils::RunInThread(m_pContext->thread(), [device = m_device, bClockwise, dSpeed]() mutable {
    if (device.isValid())
    {
      device.sendRotateCmd(bClockwise, dSpeed, -1);
    }
  });
}

//----------------------------------------------------------------------------------------
//
void CButtplugDeviceWrapper::SendStopCmd()
{
  if (!m_bDeviceValid.load()) { return; }

  utils::RunInThread(m_pContext->thread(), [device = m_device]() mutable {
    if (device.isValid())
    {
      device.sendStopDeviceCmd();
    }
  });
}

//----------------------------------------------------------------------------------------
//
void CButtplugDeviceWrapper::SendVibrateCmd(double dSpeed)
{
  if (!m_bDeviceValid.load()) { return; }

  utils::RunInThread(m_pContext->thread(), [device = m_device, dSpeed]() mutable {
    if (device.isValid())
    {
      device.sendVibrateCmd(dSpeed, -1);
    }
  });
}
