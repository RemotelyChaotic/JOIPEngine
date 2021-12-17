#include "ProjectNotificationManager.h"

#include <QJSEngine>

CNotificationInstanceMessageSender::CNotificationInstanceMessageSender() :
  QObject()
{

}
CNotificationInstanceMessageSender::~CNotificationInstanceMessageSender()
{

}

//----------------------------------------------------------------------------------------
//
CNotificationInstanceWrapper::CNotificationInstanceWrapper(QPointer<QJSEngine> pEngine,
                                                           const QString& sId) :
  CProjectEventTargetWrapper(nullptr),
  m_spMsgSender(std::make_unique<CNotificationInstanceMessageSender>()),
  m_pEngine(pEngine),
  m_sId(sId)
{
}
CNotificationInstanceWrapper::~CNotificationInstanceWrapper()
{
}

//----------------------------------------------------------------------------------------
//
void CNotificationInstanceWrapper::Dispatched(const QString&)
{

}

//----------------------------------------------------------------------------------------
//
QString CNotificationInstanceWrapper::EventTarget()
{
  return "CNotificationInstanceWrapper::" + m_sId;
}

//----------------------------------------------------------------------------------------
//
void CNotificationInstanceWrapper::InitializeEventRegistry(std::weak_ptr<CProjectEventCallbackRegistry> wpRegistry)
{
  CProjectEventTargetWrapper::InitializeEventRegistry(wpRegistry);
}

//----------------------------------------------------------------------------------------
//
void CNotificationInstanceWrapper::setTitle(const QString& sTitle)
{
  m_spMsgSender->SignalSetTitle(m_sId, sTitle);
}

//----------------------------------------------------------------------------------------
//
void CNotificationInstanceWrapper::remove()
{
  m_spMsgSender->SignalRemove(m_sId);
}

//----------------------------------------------------------------------------------------
//
CProjectNotificationManager::CProjectNotificationManager(QPointer<QJSEngine> pEngine,
                                                         QObject* pParent) :
  CProjectEventTargetWrapper(pParent),
  m_pEngine(pEngine),
  m_registry()
{
}
CProjectNotificationManager::~CProjectNotificationManager()
{
}

//----------------------------------------------------------------------------------------
//
void CProjectNotificationManager::Dispatched(const QString&)
{

}

//----------------------------------------------------------------------------------------
//
QString CProjectNotificationManager::EventTarget()
{
  return "CProjectNotificationManager";
}

//----------------------------------------------------------------------------------------
//
void CProjectNotificationManager::Initalize(std::weak_ptr<CProjectEventCallbackRegistry> wpRegistry)
{
  InitializeEventRegistry(wpRegistry);
}

//----------------------------------------------------------------------------------------
//
void CProjectNotificationManager::clearRegistry()
{
  m_registry.clear();
}

//----------------------------------------------------------------------------------------
//
void CProjectNotificationManager::deRregisterId(QString sId)
{
  auto it = m_registry.find(sId);
  if (m_registry.end() != it)
  {
    m_registry.erase(it);
  }
}

//----------------------------------------------------------------------------------------
//
QJSValue CProjectNotificationManager::get(QString sSounId)
{
  QJSValue val;
  auto it = m_registry.find(sSounId);
  if (m_registry.end() != it)
  {
    CNotificationInstanceWrapper* pWrapper = new CNotificationInstanceWrapper(m_pEngine,
                                                                              sSounId);
    pWrapper->InitializeEventRegistry(m_wpRegistry);

    connect(pWrapper->m_spMsgSender.get(), &CNotificationInstanceMessageSender::SignalRemove,
            this, &CProjectNotificationManager::signalRemove);
    connect(pWrapper->m_spMsgSender.get(), &CNotificationInstanceMessageSender::SignalSetTitle,
            this, &CProjectNotificationManager::signalSetTitle);
    connect(pWrapper->m_spMsgSender.get(), &CNotificationInstanceMessageSender::SignalRemove,
            this, &CProjectNotificationManager::SlotDestroy);

    val = m_pEngine->newQObject(pWrapper);
  }
  return val;
}

//----------------------------------------------------------------------------------------
//
void CProjectNotificationManager::registerId(QString sId)
{
  auto it = m_registry.find(sId);
  if (m_registry.end() == it)
  {
    m_registry.insert({sId, sId});
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectNotificationManager::SlotDestroy(const QString& sId)
{
  deRregisterId(sId);
}
