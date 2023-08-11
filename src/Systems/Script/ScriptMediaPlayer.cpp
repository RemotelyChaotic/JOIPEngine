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

#include "Systems/Project.h"
#include "Systems/Resource.h"

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
std::shared_ptr<CScriptObjectBase> CMediaPlayerSignalEmitter::CreateNewScriptObject(QPointer<QJSEngine> pEngine)
{
  return std::make_shared<CScriptMediaPlayer>(this, pEngine);
}
std::shared_ptr<CScriptObjectBase> CMediaPlayerSignalEmitter::CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser)
{
  return std::make_shared<CEosScriptMediaPlayer>(this, pParser);
}
std::shared_ptr<CScriptObjectBase> CMediaPlayerSignalEmitter::CreateNewScriptObject(QtLua::State* pState)
{
  return std::make_shared<CScriptMediaPlayer>(this, pState);
}

//----------------------------------------------------------------------------------------
//
CScriptMediaPlayer::CScriptMediaPlayer(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                                       QPointer<QJSEngine> pEngine) :
  CJsScriptObjectBase(pEmitter, pEngine),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
{
}
CScriptMediaPlayer::CScriptMediaPlayer(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                                       QtLua::State* pState) :
  CJsScriptObjectBase(pEmitter, pState),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
{
}

CScriptMediaPlayer::~CScriptMediaPlayer()
{
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::show(QVariant resource)
{
  if (!CheckIfScriptCanRun()) { return; }

  auto spSignalEmitter = SignalEmitter<CMediaPlayerSignalEmitter>();

  bool bOk = false;
  QString sResource = GetResourceName(resource, "show", false, &bOk);
  if (!bOk) { return; }

  emit spSignalEmitter->showMedia(sResource);
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::play()
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CMediaPlayerSignalEmitter>()->playMedia(QString(), 1, 0, -1);
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

  auto pSignalEmitter = SignalEmitter<CMediaPlayerSignalEmitter>();
  bool bOk = false;
  QString sResource = GetResourceName(resource, "play", true, &bOk);
  if (!bOk) { return; }

  emit pSignalEmitter->playMedia(sResource, iLoops, iStartAt, iEndAt);
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::playVideo()
{
  if (!CheckIfScriptCanRun()) { return; }

  emit SignalEmitter<CMediaPlayerSignalEmitter>()->playVideo();
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::seek(QVariant resource, qint64 iSeek)
{
  if (!CheckIfScriptCanRun()) { return; }

  auto pSignalEmitter = SignalEmitter<CMediaPlayerSignalEmitter>();
  bool bOk = false;
  QString sResource = GetResourceName(resource, "seek", true, &bOk);
  if (!bOk) { return; }

  emit pSignalEmitter->seekMedia(sResource, iSeek);
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::pauseVideo()
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CMediaPlayerSignalEmitter>()->pauseVideo();
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::seekVideo(qint64 iSeek)
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CMediaPlayerSignalEmitter>()->seekVideo(iSeek);
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::stopVideo()
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CMediaPlayerSignalEmitter>()->stopVideo();
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

  bool bOk = false;
  QString sResource = GetResourceName(resource, "playSound", false, &bOk);
  if (!bOk) { return; }

  emit SignalEmitter<CMediaPlayerSignalEmitter>()->playSound(sResource, sId, iLoops, iStartAt, iEndAt);
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::pauseSound(QVariant resource)
{
  if (!CheckIfScriptCanRun()) { return; }

  bool bOk = false;
  QString sResource = GetResourceName(resource, "pauseSound", true, &bOk);
  if (!bOk) { return; }

  emit SignalEmitter<CMediaPlayerSignalEmitter>()->pauseSound(sResource);
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::seekSound(QVariant resource, qint64 iSeek)
{
  if (!CheckIfScriptCanRun()) { return; }

  bool bOk = false;
  QString sResource = GetResourceName(resource, "seekSound", true, &bOk);
  if (!bOk) { return; }

  emit SignalEmitter<CMediaPlayerSignalEmitter>()->seekAudio(sResource, iSeek);
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::stopSound(QVariant resource)
{
  if (!CheckIfScriptCanRun()) { return; }

  bool bOk = false;
  QString sResource = GetResourceName(resource, "stopSound", true, &bOk);
  if (!bOk) { return; }

  emit SignalEmitter<CMediaPlayerSignalEmitter>()->stopSound(sResource);
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::setVolume(double dValue)
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CMediaPlayerSignalEmitter>()->setVolume(QString(), dValue);
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::setVolume(QVariant resource, double dValue)
{
  if (!CheckIfScriptCanRun()) { return; }

  bool bOk = false;
  QString sResource = GetResourceName(resource, "setVolume", true, &bOk);
  if (!bOk) { return; }

  emit SignalEmitter<CMediaPlayerSignalEmitter>()->setVolume(sResource, dValue);
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

  auto pSignalEmitter = SignalEmitter<CMediaPlayerSignalEmitter>();

  connect(pSignalEmitter, &CMediaPlayerSignalEmitter::videoFinished,
          pSignalEmitter, &CMediaPlayerSignalEmitter::stopVideo, Qt::DirectConnection);

  QEventLoop loop;
  connect(pSignalEmitter, &CMediaPlayerSignalEmitter::videoFinished,
          &loop, &QEventLoop::quit, Qt::QueuedConnection);
  connect(pSignalEmitter, &CMediaPlayerSignalEmitter::interrupt,
          &loop, &QEventLoop::quit, Qt::QueuedConnection);
  QMetaObject::Connection interruptThisLoop =
    connect(this, &CScriptObjectBase::SignalInterruptExecution,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
  emit pSignalEmitter->startVideoWait();
  loop.exec();

  disconnect(pSignalEmitter, &CMediaPlayerSignalEmitter::videoFinished,
             &loop, &QEventLoop::quit);
  disconnect(pSignalEmitter, &CMediaPlayerSignalEmitter::interrupt,
             &loop, &QEventLoop::quit);
  disconnect(interruptThisLoop);
  loop.disconnect();

  disconnect(pSignalEmitter, &CMediaPlayerSignalEmitter::videoFinished,
             pSignalEmitter, &CMediaPlayerSignalEmitter::stopVideo);
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
QString CScriptMediaPlayer::GetResourceName(const QVariant& resource, const QString& sMethod,
                                            bool bStringCanBeId, bool* pbOk)
{
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
    emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
    return QString();
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::WaitForPlayBackImpl(const QString& sResource)
{
  auto pSignalEmitter = SignalEmitter<CMediaPlayerSignalEmitter>();

  QMetaObject::Connection connStopPlayback =
    connect(pSignalEmitter, &CMediaPlayerSignalEmitter::playbackFinished,
            pSignalEmitter, [pSignalEmitter](const QString& sResourceRet) {
    if (sResourceRet.isEmpty())
    {
      emit pSignalEmitter->stopVideo();
    }
  }, Qt::DirectConnection);
  QMetaObject::Connection connStopSound =
    connect(pSignalEmitter, &CMediaPlayerSignalEmitter::playbackFinished,
            pSignalEmitter, [pSignalEmitter, &sResource](const QString& sResourceRet) {
    if (sResource == sResourceRet && !sResourceRet.isEmpty())
    {
      emit pSignalEmitter->stopSound(sResourceRet);
    }
  }, Qt::DirectConnection);

  QEventLoop loop;
  QMetaObject::Connection connQuit =
    connect(pSignalEmitter, &CMediaPlayerSignalEmitter::playbackFinished,
          &loop, [&loop, &sResource](const QString& sResourceRet) {
    if (sResource == sResourceRet || sResourceRet.isEmpty())
    {
      loop.quit();
    }
  }, Qt::QueuedConnection);
  connect(pSignalEmitter, &CMediaPlayerSignalEmitter::interrupt,
          &loop, &QEventLoop::quit, Qt::QueuedConnection);
  QMetaObject::Connection interruptThisLoop =
    connect(this, &CScriptObjectBase::SignalInterruptExecution,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
  emit pSignalEmitter->startPlaybackWait(sResource);
  loop.exec();

  disconnect(connQuit);
  disconnect(pSignalEmitter, &CMediaPlayerSignalEmitter::interrupt,
             &loop, &QEventLoop::quit);
  disconnect(interruptThisLoop);
  loop.disconnect();

  disconnect(connStopPlayback);
  disconnect(connStopSound);
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::WaitForSoundImpl(const QString& sResource)
{
  auto pSignalEmitter = SignalEmitter<CMediaPlayerSignalEmitter>();

  QMetaObject::Connection connStop =
    connect(pSignalEmitter, &CMediaPlayerSignalEmitter::soundFinished,
            pSignalEmitter, [pSignalEmitter, &sResource](const QString& sResourceRet) {
    if (sResource == sResourceRet && !sResourceRet.isEmpty())
    {
      emit pSignalEmitter->stopSound(sResourceRet);
    }
  }, Qt::DirectConnection);

  QEventLoop loop;
  QMetaObject::Connection connQuit =
      connect(pSignalEmitter, &CMediaPlayerSignalEmitter::soundFinished,
          &loop, [&loop, &sResource](const QString& sResourceRet) {
    if (sResource == sResourceRet || sResourceRet.isEmpty())
    {
      loop.quit();
    }
  }, Qt::QueuedConnection);
  connect(pSignalEmitter, &CMediaPlayerSignalEmitter::interrupt,
          &loop, &QEventLoop::quit, Qt::QueuedConnection);
  QMetaObject::Connection interruptThisLoop =
    connect(this, &CScriptObjectBase::SignalInterruptExecution,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
  emit pSignalEmitter->startSoundWait(sResource);
  loop.exec();

  disconnect(connQuit);
  disconnect(pSignalEmitter, &CMediaPlayerSignalEmitter::interrupt,
             &loop, &QEventLoop::quit);
  disconnect(interruptThisLoop);
  loop.disconnect();

  disconnect(connStop);
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
CEosScriptMediaPlayer::CEosScriptMediaPlayer(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                                             QPointer<CJsonInstructionSetParser> pParser) :
  CEosScriptObjectBase(pEmitter, pParser),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_spCommandImg(std::make_shared<CCommandEosImage>(this)),
  m_spCommandAudio(std::make_shared<CCommandEosAudio>(this))
{
  pParser->RegisterInstruction(eos::c_sCommandImage, m_spCommandImg);
  pParser->RegisterInstruction(eos::c_sCommandAudioPlay, m_spCommandAudio);
}
CEosScriptMediaPlayer::~CEosScriptMediaPlayer()
{
}

//----------------------------------------------------------------------------------------
//
void CEosScriptMediaPlayer::show(const QString& sResourceLocator)
{
  if (!CheckIfScriptCanRun()) { return; }

  auto spSignalEmitter = SignalEmitter<CMediaPlayerSignalEmitter>();

  bool bOk = false;
  QString sResource = GetResourceName(sResourceLocator, &bOk);
  if (!bOk) { return; }

  emit spSignalEmitter->showMedia(sResource);
}

//----------------------------------------------------------------------------------------
//
void CEosScriptMediaPlayer::playSound(const QString& sResourceLocator, const QString& iId,
                                      qint64 iLoops, qint64 iStartAt)
{
  if (!CheckIfScriptCanRun()) { return; }

  auto spSignalEmitter = SignalEmitter<CMediaPlayerSignalEmitter>();

  bool bOk = false;
  QString sResource = GetResourceName(sResourceLocator, &bOk);
  if (!bOk) { return; }

  emit spSignalEmitter->playSound(sResource, iId, iLoops, iStartAt, -1);
}

//----------------------------------------------------------------------------------------
//
void CEosScriptMediaPlayer::seek(const QString& iId, qint64 iSeek)
{
  if (!CheckIfScriptCanRun()) { return; }
  auto spSignalEmitter = SignalEmitter<CMediaPlayerSignalEmitter>();
  emit spSignalEmitter->seekMedia(iId, iSeek);
}

//----------------------------------------------------------------------------------------
//
void CEosScriptMediaPlayer::setVolume(const QString& iId, double dValue)
{
  if (!CheckIfScriptCanRun()) { return; }
  auto spSignalEmitter = SignalEmitter<CMediaPlayerSignalEmitter>();
  emit spSignalEmitter->setVolume(iId, dValue);
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
