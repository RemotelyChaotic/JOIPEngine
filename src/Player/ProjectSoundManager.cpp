#include "ProjectSoundManager.h"
#include "Systems/Database/Resource.h"
#include "Systems/Script/ScriptDbWrappers.h"

#include <QJSEngine>

//----------------------------------------------------------------------------------------
//
CSoundInstanceMessageSender::CSoundInstanceMessageSender() :
  QObject()
{
}
CSoundInstanceMessageSender::~CSoundInstanceMessageSender()
{
}

//----------------------------------------------------------------------------------------
//
CSoundInstanceWrapper::CSoundInstanceWrapper(QPointer<QJSEngine> pEngine,
                                             const std::shared_ptr<SResource>& spResource,
                                             const QString& sId,
                                             qint32 iLoops,
                                             qint32 iStartAt,
                                             qint32 iEndAt) :
  CProjectEventTargetWrapper(nullptr),
  m_spMsgSender(std::make_unique<CSoundInstanceMessageSender>()),
  m_spResource(spResource),
  m_pEngine(pEngine),
  m_sId(sId),
  m_iLoops(iLoops),
  m_iStartAt(iStartAt),
  m_iEndAt(iEndAt)
{

}
CSoundInstanceWrapper::~CSoundInstanceWrapper()
{

}

//----------------------------------------------------------------------------------------
//
void CSoundInstanceWrapper::Dispatched(const QString&)
{

}

//----------------------------------------------------------------------------------------
//
QString CSoundInstanceWrapper::EventTarget()
{
  return "CSoundInstanceWrapper::" + m_sId;
}

//----------------------------------------------------------------------------------------
//
void CSoundInstanceWrapper::InitializeEventRegistry(std::weak_ptr<CProjectEventCallbackRegistry> wpRegistry)
{
  CProjectEventTargetWrapper::InitializeEventRegistry(wpRegistry);
  if (auto spRegistry = m_wpRegistry.lock())
  {
    spRegistry->AddDispatchTarget({EventTarget(), "play"}, this);
    spRegistry->AddDispatchTarget({EventTarget(), "end"}, this);
    spRegistry->AddDispatchTarget({EventTarget(), "pause"}, this);
  }
}

//----------------------------------------------------------------------------------------
//
void CSoundInstanceWrapper::play()
{
  QString sName;
  {
    QReadLocker locker(&m_spResource->m_rwLock);
    sName = m_spResource->m_sName;
  }
  emit m_spMsgSender->SignalPlay(m_sId, sName, m_iLoops, m_iStartAt, m_iEndAt);
}

//----------------------------------------------------------------------------------------
//
void CSoundInstanceWrapper::pause()
{
  emit m_spMsgSender->SignalPause(m_sId);
}

//----------------------------------------------------------------------------------------
//
void CSoundInstanceWrapper::stop()
{
  emit m_spMsgSender->SignalStop(m_sId);
}

//----------------------------------------------------------------------------------------
//
void CSoundInstanceWrapper::seek(double dTime)
{
  emit m_spMsgSender->SignalSeek(m_sId, dTime);
}

//----------------------------------------------------------------------------------------
//
void CSoundInstanceWrapper::setVolume(double dVal)
{
  emit m_spMsgSender->SignalSetVolume(m_sId, dVal);
}

//----------------------------------------------------------------------------------------
//
void CSoundInstanceWrapper::destroy()
{
  emit m_spMsgSender->SignalDestroy(m_sId);
}

//----------------------------------------------------------------------------------------
//
CProjectSoundManager::CProjectSoundManager(QPointer<QJSEngine> pEngine,
                                           QObject* pParent) :
  CProjectEventTargetWrapper(pParent),
  m_pEngine(pEngine),
  m_registry()
{

}
CProjectSoundManager::~CProjectSoundManager()
{
  clearRegistry();
}

//----------------------------------------------------------------------------------------
//
void CProjectSoundManager::Dispatched(const QString&)
{
}

//----------------------------------------------------------------------------------------
//
QString CProjectSoundManager::EventTarget()
{
  return "CProjectSoundManager";
}

//----------------------------------------------------------------------------------------
//
void CProjectSoundManager::Initalize(std::weak_ptr<CProjectEventCallbackRegistry> wpRegistry)
{
  InitializeEventRegistry(wpRegistry);
}

//----------------------------------------------------------------------------------------
//
void CProjectSoundManager::clearRegistry()
{
  m_registry.clear();
}

//----------------------------------------------------------------------------------------
//
void CProjectSoundManager::deRregisterId(QString sId)
{
  auto it = m_registry.find(sId);
  if (m_registry.end() != it)
  {
    m_registry.erase(it);
  }
}

//----------------------------------------------------------------------------------------
//
QJSValue CProjectSoundManager::get(QString sSounId)
{
  QJSValue val;
  auto it = m_registry.find(sSounId);
  if (m_registry.end() != it)
  {
    CSoundInstanceWrapper* pWrapper = new CSoundInstanceWrapper(m_pEngine,
                                                                it->second.m_spResource,
                                                                sSounId,
                                                                it->second.m_iLoops,
                                                                it->second.m_iStartAt,
                                                                it->second.m_iEndAt);
    pWrapper->InitializeEventRegistry(m_wpRegistry);

    connect(pWrapper->m_spMsgSender.get(), &CSoundInstanceMessageSender::SignalDestroy,
            this, &CProjectSoundManager::SlotDestroy);
    connect(pWrapper->m_spMsgSender.get(), &CSoundInstanceMessageSender::SignalPlay,
            this, &CProjectSoundManager::signalPlay);
    connect(pWrapper->m_spMsgSender.get(), &CSoundInstanceMessageSender::SignalPause,
            this, &CProjectSoundManager::signalPause);
    connect(pWrapper->m_spMsgSender.get(), &CSoundInstanceMessageSender::SignalSeek,
            this, &CProjectSoundManager::signalSeek);
    connect(pWrapper->m_spMsgSender.get(), &CSoundInstanceMessageSender::SignalSetVolume,
            this, &CProjectSoundManager::signalSetVolume);
    connect(pWrapper->m_spMsgSender.get(), &CSoundInstanceMessageSender::SignalStop,
            this, &CProjectSoundManager::signalStop);

    val = m_pEngine->newQObject(pWrapper);
  }
  return val;
}

//----------------------------------------------------------------------------------------
//
void CProjectSoundManager::registerId(QString sId, QJSValue sound,
                                      qint32 iLoops, qint32 iStartAt,
                                      qint32 iEndAt)
{
  auto it = m_registry.find(sId);
  if (m_registry.end() == it && sound.isObject())
  {
    CResourceScriptWrapper* pResource =
        dynamic_cast<CResourceScriptWrapper*>(sound.toQObject());
    if (nullptr != pResource)
    {
      m_registry.insert({sId, {pResource->Data(), iLoops, iStartAt, iEndAt}});
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectSoundManager::SlotDestroy(const QString& sId)
{
  deRregisterId(sId);
}
