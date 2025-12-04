#include "ScriptMediaPlayer.h"
#include "Application.h"
#include "CommonScriptHelpers.h"
#include "ScriptDbWrappers.h"

#include "Systems/DatabaseManager.h"

#include "Systems/EOS/CommandEosAudioBase.h"
#include "Systems/EOS/CommandEosImageBase.h"
#include "Systems/EOS/EosCommands.h"
#include "Systems/EOS/EosHelpers.h"

#include "Systems/JSON/JsonInstructionBase.h"
#include "Systems/JSON/JsonInstructionSetParser.h"

#include "Systems/Sequence/SequenceMediaPlayerRunner.h"

#include "Systems/Database/Project.h"
#include "Systems/Database/Resource.h"

#include <QTimer>
#include <chrono>
#include <random>

CMediaPlayerSignalEmitter::CMediaPlayerSignalEmitter() :
  CScriptRunnerSignalEmiter()
{

}
CMediaPlayerSignalEmitter::~CMediaPlayerSignalEmitter()
{

}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CScriptCommunicator>
CMediaPlayerSignalEmitter::CreateCommunicatorImpl(std::shared_ptr<CScriptRunnerSignalEmiterAccessor> spAccessor)
{
  return std::make_shared<CMediaPlayerScriptCommunicator>(spAccessor);
}

//----------------------------------------------------------------------------------------
//
CMediaPlayerScriptCommunicator::CMediaPlayerScriptCommunicator(
  const std::weak_ptr<CScriptRunnerSignalEmiterAccessor>& spEmitter) :
  CScriptCommunicator(spEmitter)
{}
CMediaPlayerScriptCommunicator::~CMediaPlayerScriptCommunicator() = default;

//----------------------------------------------------------------------------------------
//
CScriptObjectBase* CMediaPlayerScriptCommunicator::CreateNewScriptObject(QPointer<QJSEngine> pEngine)
{
  return new CScriptMediaPlayer(weak_from_this(), pEngine);
}
CScriptObjectBase* CMediaPlayerScriptCommunicator::CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser)
{
  return new CEosScriptMediaPlayer(weak_from_this(), pParser);
}
CScriptObjectBase* CMediaPlayerScriptCommunicator::CreateNewScriptObject(QtLua::State* pState)
{
  return new CScriptMediaPlayer(weak_from_this(), pState);
}
CScriptObjectBase* CMediaPlayerScriptCommunicator::CreateNewSequenceObject()
{
  return new CSequenceMediaPlayerRunner(weak_from_this());
}

//----------------------------------------------------------------------------------------
//
CScriptMediaPlayer::CScriptMediaPlayer(std::weak_ptr<CScriptCommunicator> pCommunicator,
                                       QPointer<QJSEngine> pEngine) :
  CJsScriptObjectBase(pCommunicator, pEngine),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
{
  m_spStop = std::make_shared<std::function<void()>>([this]() {
    emit SignalQuitLoop();
  });
  if (auto spComm = pCommunicator.lock())
  {
    spComm->RegisterStopCallback(m_spStop);

    if (auto spSignalEmitter = spComm->LockedEmitter<CMediaPlayerSignalEmitter>())
    {
      connect(spSignalEmitter.Get(), &CMediaPlayerSignalEmitter::soundFinished, this,
          [this](const QString& sResource) {
            HandleMediaStateChange(sResource, EStateChange::eSound);
          }, Qt::QueuedConnection);
      connect(spSignalEmitter.Get(), &CMediaPlayerSignalEmitter::videoFinished, this,
          [this](const QString& sResource) {
            HandleMediaStateChange(sResource, EStateChange::eVideo);
          }, Qt::QueuedConnection);
    }
  }
}
CScriptMediaPlayer::CScriptMediaPlayer(std::weak_ptr<CScriptCommunicator> pCommunicator,
                                       QtLua::State* pState) :
  CJsScriptObjectBase(pCommunicator, pState),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
{
  m_spStop = std::make_shared<std::function<void()>>([this]() {
    emit SignalQuitLoop();
  });
  if (auto spComm = pCommunicator.lock())
  {
    spComm->RegisterStopCallback(m_spStop);

    if (auto spSignalEmitter = spComm->LockedEmitter<CMediaPlayerSignalEmitter>())
    {
      connect(spSignalEmitter.Get(), &CMediaPlayerSignalEmitter::soundFinished, this,
          [this](const QString& sResource) {
            HandleMediaStateChange(sResource, EStateChange::eSound);
          }, Qt::QueuedConnection);
      connect(spSignalEmitter.Get(), &CMediaPlayerSignalEmitter::videoFinished, this,
          [this](const QString& sResource) {
            HandleMediaStateChange(sResource, EStateChange::eVideo);
          }, Qt::QueuedConnection);
    }
  }
}

CScriptMediaPlayer::~CScriptMediaPlayer()
{
  if (auto spComm = m_wpCommunicator.lock())
  {
    spComm->RemoveStopCallback(m_spStop);
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::show(QVariant resource)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CMediaPlayerSignalEmitter>())
    {
      bool bOk = false;
      QString sResource = GetResourceName(resource, "show", false, &bOk);
      if (!bOk) { return; }

      emit spSignalEmitter->showMedia(sResource);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::play()
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CMediaPlayerSignalEmitter>())
    {
      emit spSignalEmitter->playMedia(QString(), 1, 0, -1);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::play(QVariant resource)
{
  play(resource, 1, 0, -1);
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::play(QVariant resource, qint64 iLoops)
{
  play(resource, iLoops, 0, -1);
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::play(QVariant resource, qint64 iLoops, qint64 iStartAt)
{
  play(resource, iLoops, iStartAt, -1);
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::play(QVariant resource, qint64 iLoops, qint64 iStartAt, qint64 iEndAt)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CMediaPlayerSignalEmitter>())
    {
      bool bOk = false;
      QString sResource = GetResourceName(resource, "play", true, &bOk);
      if (!bOk) { return; }

      emit spSignalEmitter->playMedia(sResource, iLoops, iStartAt, iEndAt);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::playVideo()
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CMediaPlayerSignalEmitter>())
    {
      emit spSignalEmitter->playVideo();
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::seek(QVariant resource, qint64 iSeek)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CMediaPlayerSignalEmitter>())
    {
      bool bOk = false;
      QString sResource = GetResourceName(resource, "seek", true, &bOk);
      if (!bOk) { return; }

      emit spSignalEmitter->seekMedia(sResource, iSeek);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::pauseVideo()
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CMediaPlayerSignalEmitter>())
    {
      emit spSignalEmitter->pauseVideo();
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::seekVideo(qint64 iSeek)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CMediaPlayerSignalEmitter>())
    {
      emit spSignalEmitter->seekVideo(iSeek);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::stopVideo()
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CMediaPlayerSignalEmitter>())
    {
      emit spSignalEmitter->stopVideo();
    }
  }
}


//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::playSound(QVariant resource)
{
  playSound(resource, QString(), 1, 0, -1);
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::playSound(QVariant resource, const QString& sId)
{
  playSound(resource, sId, 1, 0, -1);
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::playSound(QVariant resource, const QString& sId, qint64 iLoops)
{
  playSound(resource, sId, iLoops, 0, -1);
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::playSound(QVariant resource, const QString& sId, qint64 iLoops, qint64 iStartAt)
{
  playSound(resource, sId, iLoops, iStartAt, -1);
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::playSound(QVariant resource, const QString& sId, qint64 iLoops,
                                   qint64 iStartAt, qint64 iEndAt)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CMediaPlayerSignalEmitter>())
    {
      bool bOk = false;
      QString sResource = GetResourceName(resource, "playSound", false, &bOk);
      if (!bOk) { return; }

      emit spSignalEmitter->playSound(sResource, sId, iLoops, iStartAt, iEndAt);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::pauseSound(QVariant resource)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CMediaPlayerSignalEmitter>())
    {
      bool bOk = false;
      QString sResource = GetResourceName(resource, "pauseSound", true, &bOk);
      if (!bOk) { return; }

      emit spSignalEmitter->pauseSound(sResource);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::seekSound(QVariant resource, qint64 iSeek)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CMediaPlayerSignalEmitter>())
    {
      bool bOk = false;
      QString sResource = GetResourceName(resource, "seekSound", true, &bOk);
      if (!bOk) { return; }

      emit spSignalEmitter->seekAudio(sResource, iSeek);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::stopSound(QVariant resource)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CMediaPlayerSignalEmitter>())
    {
      bool bOk = false;
      QString sResource = GetResourceName(resource, "stopSound", true, &bOk);
      if (!bOk) { return; }

      emit spSignalEmitter->stopSound(sResource);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::setVolume(double dValue)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CMediaPlayerSignalEmitter>())
    {
      emit spSignalEmitter->setVolume(QString(), dValue);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::setVolume(QVariant resource, double dValue)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CMediaPlayerSignalEmitter>())
    {
      bool bOk = false;
      QString sResource = GetResourceName(resource, "setVolume", true, &bOk);
      if (!bOk) { return; }

      emit spSignalEmitter->setVolume(sResource, dValue);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::waitForPlayback()
{
  if (!CheckIfScriptCanRun()) { return; }
  WaitForPlayBackImpl(QString());
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::waitForPlayback(QVariant resource)
{
  if (!CheckIfScriptCanRun()) { return; }

  bool bOk = false;
  QString sResource = GetResourceName(resource, "waitForPlayback", true, &bOk);
  if (!bOk) { return; }

  WaitForPlayBackImpl(sResource);
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::waitForVideo()
{
  if (!CheckIfScriptCanRun()) { return; }

  QTimer::singleShot(0, this, [this]() {
    if (auto spComm = m_wpCommunicator.lock())
    {
      if (auto spSignalEmitter = spComm->LockedEmitter<CMediaPlayerSignalEmitter>())
      {
        emit spSignalEmitter->startVideoWait();
      }
    }
  });

  QPointer<CScriptMediaPlayer> pThis(this);
  QEventLoop loop;

  QMetaObject::Connection finishedConn;
  QMetaObject::Connection finishedLoopConn;
  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CMediaPlayerSignalEmitter>())
    {
      finishedConn =
        connect(spSignalEmitter.Get(), &CMediaPlayerSignalEmitter::videoFinished,
                spSignalEmitter.Get(), &CMediaPlayerSignalEmitter::stopVideo,
                Qt::DirectConnection);


      finishedLoopConn =
        connect(spSignalEmitter.Get(), &CMediaPlayerSignalEmitter::videoFinished,
                &loop, &QEventLoop::quit, Qt::QueuedConnection);
    }
  }

  QMetaObject::Connection quitLoop =
    connect(this, &CScriptMediaPlayer::SignalQuitLoop, &loop, &QEventLoop::quit,
            Qt::QueuedConnection);
  QMetaObject::Connection interruptThisLoop =
    connect(this, &CScriptObjectBase::SignalInterruptExecution,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
  loop.exec();
  loop.disconnect();

  if (nullptr != pThis)
  {
    disconnect(interruptThisLoop);
    disconnect(quitLoop);
    if (finishedLoopConn) disconnect(finishedLoopConn);
  }

  if (finishedConn) QObject::disconnect(finishedConn);
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::waitForSound()
{
  if (!CheckIfScriptCanRun()) { return; }
  WaitForSoundImpl(QString());
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::waitForSound(QVariant resource)
{
  if (!CheckIfScriptCanRun()) { return; }

  bool bOk = false;
  QString sResource = GetResourceName(resource, "waitForSound", true, &bOk);
  if (!bOk) { return; }

  WaitForSoundImpl(sResource);
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::registerMediaCallback(const QVariant& resource, QVariant callback)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CMediaPlayerSignalEmitter>())
    {
      auto spDbManager = m_wpDbManager.lock();
      QString sError;
      std::optional<QString> optRes =
          script::ParseResourceFromScriptVariant(resource, m_wpDbManager.lock(),
                                                 m_spProject,
                                                 "registerMediaCallback", &sError);
      if (!optRes.has_value())
      {
        emit spSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
        return;
      }

      QString resRet = optRes.value();

      if (callback.canConvert<QtLua::Value>())
      {
        QtLua::Value fn = callback.value<QtLua::Value>();
        if (fn.type() == QtLua::Value::TFunction)
        {
          m_callbacksSound[resRet] = fn;
          m_callbacksVideo[resRet] = fn;
        }
      }
      else if (callback.canConvert<QJSValue>())
      {
        QJSValue fn = callback.value<QJSValue>();
        if (fn.isCallable())
        {
          m_callbacksSound[resRet] = fn;
          m_callbacksVideo[resRet] = fn;
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::registerSoundCallback(const QVariant& resourceOrId, QVariant callback)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CMediaPlayerSignalEmitter>())
    {
      auto spDbManager = m_wpDbManager.lock();
      QString sError;
      std::optional<QString> optRes =
          resourceOrId.type() == QVariant::String ? resourceOrId.toString() :
          script::ParseResourceFromScriptVariant(resourceOrId, m_wpDbManager.lock(),
                                                 m_spProject,
                                                 "registerSoundCallback", &sError);
      if (!optRes.has_value())
      {
        emit spSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
        return;
      }

      QString resRet = optRes.value();

      if (callback.canConvert<QtLua::Value>())
      {
        QtLua::Value fn = callback.value<QtLua::Value>();
        if (fn.type() == QtLua::Value::TFunction)
        {
          m_callbacksSound[resRet] = fn;
        }
      }
      else if (callback.canConvert<QJSValue>())
      {
        QJSValue fn = callback.value<QJSValue>();
        if (fn.isCallable())
        {
          m_callbacksSound[resRet] = fn;
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::registerVideoCallback(const QVariant& resource, QVariant callback)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CMediaPlayerSignalEmitter>())
    {
      auto spDbManager = m_wpDbManager.lock();
      QString sError;
      std::optional<QString> optRes =
          script::ParseResourceFromScriptVariant(resource, m_wpDbManager.lock(),
                                                 m_spProject,
                                                 "registerVideoCallback", &sError);
      if (!optRes.has_value())
      {
        emit spSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
        return;
      }

      QString resRet = optRes.value();

      if (callback.canConvert<QtLua::Value>())
      {
        QtLua::Value fn = callback.value<QtLua::Value>();
        if (fn.type() == QtLua::Value::TFunction)
        {
          m_callbacksVideo[resRet] = fn;
        }
      }
      else if (callback.canConvert<QJSValue>())
      {
        QJSValue fn = callback.value<QJSValue>();
        if (fn.isCallable())
        {
          m_callbacksVideo[resRet] = fn;
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::HandleMediaStateChange(const QString& sResource, EStateChange callbackType)
{
  if (!CheckIfScriptCanRun()) { return; }

  std::vector<std::map<QString, script::tCallbackValue>*> vpMaps;
  switch (callbackType)
  {
    case eMedia: vpMaps.push_back(&m_callbacksSound); vpMaps.push_back(&m_callbacksVideo); break;
    case eSound: vpMaps.push_back(&m_callbacksSound); break;
    case eVideo: vpMaps.push_back(&m_callbacksVideo); break;
  }

  for (std::map<QString, script::tCallbackValue>* pMap : vpMaps)
  {
    QString sError;
    if (nullptr != m_pEngine)
    {
      auto it = pMap->find(sResource);
      if (pMap->end() != it && std::holds_alternative<QJSValue>(it->second))
      {
        if (!script::CallCallback(std::get<QJSValue>(it->second), QJSValueList() << QJSValue(sResource),
                                  &sError))
        {
          if (auto spComm = m_wpCommunicator.lock())
          {
            if (auto spSignalEmitter = spComm->LockedEmitter<CMediaPlayerSignalEmitter>())
            {
              emit spSignalEmitter->showError(sError, QtMsgType::QtCriticalMsg);
            }
          }
        }
      }
    }
    else if (nullptr != m_pState)
    {
      auto it = pMap->find(sResource);
      if (pMap->end() != it && std::holds_alternative<QtLua::Value>(it->second))
      {
        if (!script::CallCallback(std::get<QtLua::Value>(it->second), QVariantList() << sResource,
                                  &sError))
        {
          if (auto spComm = m_wpCommunicator.lock())
          {
            if (auto spSignalEmitter = spComm->LockedEmitter<CMediaPlayerSignalEmitter>())
            {
              emit spSignalEmitter->showError(sError, QtMsgType::QtCriticalMsg);
            }
          }
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
QString CScriptMediaPlayer::GetResourceName(const QVariant& resource, const QString& sMethod,
                                            bool bStringCanBeId, bool* pbOk)
{
  Q_UNUSED(bStringCanBeId)
  QString sError;
  std::optional<QString> optRes =
      script::ParseResourceFromScriptVariant(resource, m_wpDbManager.lock(),
                                             m_spProject,
                                             sMethod, &sError);

  if (nullptr != pbOk)
  {
    *pbOk = optRes.has_value();
  }

  if (optRes.has_value())
  {
    return optRes.value();
  }
  else
  {
    if (auto spComm = m_wpCommunicator.lock())
    {
      if (auto spSignalEmitter = spComm->LockedEmitter<CMediaPlayerSignalEmitter>())
      {
        emit spSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
      }
    }
    return QString();
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::WaitForPlayBackImpl(const QString& sResource)
{
  QPointer<CScriptMediaPlayer> pThis(this);
  QEventLoop loop;

  QString sRes = sResource;
  sRes.detach();

  QTimer::singleShot(0, this, [this, sRes]() {
    if (auto spComm = m_wpCommunicator.lock())
    {
      if (auto spSignalEmitter = spComm->LockedEmitter<CMediaPlayerSignalEmitter>())
      {
        emit spSignalEmitter->startPlaybackWait(sRes);
      }
    }
  });

  EResourceType type = EResourceType::eSound;
  if (auto spManager = m_wpDbManager.lock())
  {
    auto spRes = spManager->FindResourceInProject(m_spProject, sRes);
    QReadLocker l(&spRes->m_rwLock);
    type = spRes->m_type;
  }

  QMetaObject::Connection connStopPlayback;
  QMetaObject::Connection connFinished;
  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CMediaPlayerSignalEmitter>())
    {
      connStopPlayback =
        connect(spSignalEmitter.Get(), &CMediaPlayerSignalEmitter::playbackFinished,
                spSignalEmitter.Get(),
                [pSignalEmitter = spSignalEmitter.Get(), sRes, type](const QString& sResourceRet) {
                  if (sRes == sResourceRet && !sResourceRet.isEmpty())
                  {
                    if (EResourceType::eSound == type._to_integral())
                    {
                      emit pSignalEmitter->stopSound(sResourceRet);
                    }
                    else if (EResourceType::eMovie == type._to_integral())
                    {
                      emit pSignalEmitter->videoFinished(sResourceRet);
                    }
                  }
                  else
                  {
                    emit pSignalEmitter->videoFinished(sResourceRet);
                  }
                }, Qt::DirectConnection);
      connFinished =
        connect(spSignalEmitter.Get(), &CMediaPlayerSignalEmitter::playbackFinished,
                &loop, [&loop, sRes](const QString& sResourceRet) {
                  if (sRes == sResourceRet || sResourceRet.isEmpty() || sRes.isEmpty())
                  {
                    bool bOk = QMetaObject::invokeMethod(&loop, "quit", Qt::QueuedConnection);
                    assert(bOk); Q_UNUSED(bOk)
                  }
                }, Qt::QueuedConnection);
    }
  }

  QMetaObject::Connection connQuit =
    connect(this, &CScriptMediaPlayer::SignalQuitLoop,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
  QMetaObject::Connection interruptThisLoop =
    connect(this, &CScriptObjectBase::SignalInterruptExecution,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
  loop.exec();
  loop.disconnect();

  if (nullptr != pThis)
  {
    if (connFinished) disconnect(connFinished);
    disconnect(connQuit);
    disconnect(interruptThisLoop);
  }

  if (connStopPlayback) QObject::disconnect(connStopPlayback);
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::WaitForSoundImpl(const QString& sResource)
{
  QPointer<CScriptMediaPlayer> pThis(this);
  QEventLoop loop;
  QString sRes = sResource;
  sRes.detach();

  QTimer::singleShot(0, this, [this, sRes]() {
    if (auto spComm = m_wpCommunicator.lock())
    {
      if (auto spSignalEmitter = spComm->LockedEmitter<CMediaPlayerSignalEmitter>())
      {
        emit spSignalEmitter->startSoundWait(sRes);
      }
    }
  });

  QMetaObject::Connection connStopSound;
  QMetaObject::Connection connFinished;
  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CMediaPlayerSignalEmitter>())
    {
      connStopSound =
        connect(spSignalEmitter.Get(), &CMediaPlayerSignalEmitter::playbackFinished,
                spSignalEmitter.Get(),
                [pSignalEmitter = spSignalEmitter.Get(), sRes](const QString& sResourceRet) {
                  if (sRes == sResourceRet && !sResourceRet.isEmpty())
                  {
                    emit pSignalEmitter->stopSound(sResourceRet);
                  }
                }, Qt::DirectConnection);
      connFinished =
        connect(spSignalEmitter.Get(), &CMediaPlayerSignalEmitter::playbackFinished,
                &loop, [&loop, sRes](const QString& sResourceRet) {
                  if (sRes == sResourceRet || sResourceRet.isEmpty())
                  {
                    bool bOk = QMetaObject::invokeMethod(&loop, "quit", Qt::QueuedConnection);
                    assert(bOk); Q_UNUSED(bOk)
                  }
                }, Qt::QueuedConnection);
    }
  }

  QMetaObject::Connection connQuit =
    connect(this, &CScriptMediaPlayer::SignalQuitLoop,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
  QMetaObject::Connection interruptThisLoop =
    connect(this, &CScriptObjectBase::SignalInterruptExecution,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
  loop.exec();
  loop.disconnect();

  if (nullptr != pThis)
  {
    if (connFinished) disconnect(connFinished);
    disconnect(connQuit);
    disconnect(interruptThisLoop);
  }

  if (connStopSound) QObject::disconnect(connStopSound);
}

//----------------------------------------------------------------------------------------
//
class CCommandEosImage : public CCommandEosImageBase
{
public:
  CCommandEosImage(CEosScriptMediaPlayer* pParent) :
    CCommandEosImageBase(),
    m_pParent(pParent)
  {}
  ~CCommandEosImage() override {}

  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) override
  {
    const auto& itLocator = GetValue<EArgumentType::eString>(args, "locator");
    if (HasValue(args, "locator") && IsOk<EArgumentType::eString>(itLocator) &&
        nullptr != m_pParent)
    {
      QString sResourceLocator = std::get<QString>(itLocator);
      m_pParent->show(sResourceLocator);
      return SRunRetVal<ENextCommandToCall::eSibling>();
    }
    return SJsonException{"Locator missing from image call.", "locator", eos::c_sCommandImage, 0, 0};
  }

private:
  CEosScriptMediaPlayer* m_pParent;
};

//----------------------------------------------------------------------------------------
//
class CCommandEosAudio : public CCommandEosAudioBase
{
public:
  CCommandEosAudio(CEosScriptMediaPlayer* pParent) :
    CCommandEosAudioBase(),
    m_pParent(pParent)
  {}
  ~CCommandEosAudio() override {}

  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) override
  {
    const auto& itLocator = GetValue<EArgumentType::eString>(args, "locator");
    const auto& itId = GetValue<EArgumentType::eString>(args, "id");
    const auto& itLoops = GetValue<EArgumentType::eInt64>(args, "loops");
    const auto& itSeek = GetValue<EArgumentType::eInt64>(args, "seek");
    const auto& itStartAt = GetValue<EArgumentType::eInt64>(args, "startAt");

    const bool bHasLocator = HasValue(args, "locator") && IsOk<EArgumentType::eString>(itLocator);
    const bool bHasId = HasValue(args, "id") && IsOk<EArgumentType::eString>(itId);
    const bool bHasSeek = HasValue(args, "seek") && IsOk<EArgumentType::eInt64>(itSeek);

    if ((bHasLocator || (bHasSeek && (bHasId ||  bHasLocator))) && nullptr != m_pParent)
    {
      // play
      if (bHasLocator)
      {
        QString sResourceLocator = std::get<QString>(itLocator);
        // optional
        QString sId;
        if (bHasId)
        {
          sId = std::get<QString>(itId);
        }
        qint64 iLoops = 1;
        if (HasValue(args, "loops") && IsOk<EArgumentType::eInt64>(itLoops))
        {
          iLoops = std::get<qint64>(itLoops);
        }
        qint64 iStartAt = 0;
        if (HasValue(args, "startAt") && IsOk<EArgumentType::eInt64>(itStartAt))
        {
          iStartAt = std::get<qint64>(itStartAt);
        }

        m_pParent->playSound(sResourceLocator, sId, 0==iLoops ? -1 : iLoops, iStartAt);

        const auto& itVolume = GetValue<EArgumentType::eDouble>(args, "volume");
        if (HasValue(args, "volume") && IsOk<EArgumentType::eDouble>(itVolume))
        {
          m_pParent->setVolume(sId, std::get<double>(itVolume));
        }
        else
        {
          m_pParent->setVolume(sId, 1.0);
        }

        return SRunRetVal<ENextCommandToCall::eSibling>();
      }

      // seek
      if (bHasSeek && (bHasId ||  bHasLocator))
      {
        QString sId;
        if (bHasId)
        {
          sId = std::get<QString>(itId);
        }
        else if (bHasLocator)
        {
          sId = std::get<QString>(itLocator);
        }

        qint64 iSeek = 0;
        if (bHasSeek)
        {
          iSeek = std::get<qint64>(itSeek);
        }

        m_pParent->seek(sId, iSeek);
      }

      return SRunRetVal<ENextCommandToCall::eSibling>();
    }
    return SJsonException{"Locator or id missing from audio.play call.", "locator", eos::c_sCommandAudioPlay, 0, 0};
  }

private:
  CEosScriptMediaPlayer* m_pParent;
};

//----------------------------------------------------------------------------------------
//
CEosScriptMediaPlayer::CEosScriptMediaPlayer(std::weak_ptr<CScriptCommunicator> pCommunicator,
                                             QPointer<CJsonInstructionSetParser> pParser) :
  CEosScriptObjectBase(pCommunicator, pParser),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_spCommandImg(std::make_shared<CCommandEosImage>(this)),
  m_spCommandAudio(std::make_shared<CCommandEosAudio>(this))
{
  m_spStop = std::make_shared<std::function<void()>>([this]() {
    emit SignalQuitLoop();
  });
  if (auto spComm = pCommunicator.lock())
  {
    spComm->RegisterStopCallback(m_spStop);
  }
  pParser->RegisterInstruction(eos::c_sCommandImage, m_spCommandImg);
  pParser->RegisterInstruction(eos::c_sCommandAudioPlay, m_spCommandAudio);
}
CEosScriptMediaPlayer::~CEosScriptMediaPlayer()
{
  if (auto spComm = m_wpCommunicator.lock())
  {
    spComm->RemoveStopCallback(m_spStop);
  }
}

//----------------------------------------------------------------------------------------
//
void CEosScriptMediaPlayer::show(const QString& sResourceLocator)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CMediaPlayerSignalEmitter>())
    {
      bool bOk = false;
      QString sResource = GetResourceName(sResourceLocator, &bOk);
      if (!bOk) { return; }

      emit spSignalEmitter->showMedia(sResource);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEosScriptMediaPlayer::playSound(const QString& sResourceLocator, const QString& iId,
                                      qint64 iLoops, qint64 iStartAt)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CMediaPlayerSignalEmitter>())
    {
      bool bOk = false;
      QString sResource = GetResourceName(sResourceLocator, &bOk);
      if (!bOk) { return; }

      emit spSignalEmitter->playSound(sResource, iId, iLoops, iStartAt, -1);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEosScriptMediaPlayer::seek(const QString& iId, qint64 iSeek)
{
  if (!CheckIfScriptCanRun()) { return; }
  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CMediaPlayerSignalEmitter>())
    {
      emit spSignalEmitter->seekMedia(iId, iSeek);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEosScriptMediaPlayer::setVolume(const QString& iId, double dValue)
{
  if (!CheckIfScriptCanRun()) { return; }
  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CMediaPlayerSignalEmitter>())
    {
      emit spSignalEmitter->setVolume(iId, dValue);
    }
  }
}

//----------------------------------------------------------------------------------------
//
QString CEosScriptMediaPlayer::GetResourceName(const QString& sResourceLocator, bool* pbOk)
{
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != pbOk) { *pbOk = true; }
  QString sResource;
  if (nullptr != spDbManager)
  {
    if (eos::LookupRemoteLink(sResourceLocator, m_spProject, spDbManager, sResource))
    {
      return sResource;
    }
    if (eos::LookupGaleryImage(sResourceLocator, m_spProject, spDbManager, sResource))
    {
      return sResource;
    }
    if (eos::LookupFile(sResourceLocator, m_spProject, spDbManager, sResource))
    {
      return sResource;
    }
    if (nullptr != pbOk) { *pbOk = false; }
  }

  return sResource;
}
