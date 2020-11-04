#include "ScriptMediaPlayer.h"
#include "Application.h"
#include "Systems/DatabaseManager.h"
#include "Systems/Project.h"
#include "Systems/Resource.h"
#include <QTimer>

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

//----------------------------------------------------------------------------------------
//
CScriptMediaPlayer::CScriptMediaPlayer(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                                       QPointer<QJSEngine> pEngine) :
  CScriptObjectBase(pEmitter, pEngine),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
{
}

CScriptMediaPlayer::~CScriptMediaPlayer()
{
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::show(QJSValue resource)
{
  if (!CheckIfScriptCanRun()) { return; }

  auto spSignalEmitter = SignalEmitter<CMediaPlayerSignalEmitter>();

  bool bError = false;
  QString sResource = GetResourceName(resource, &bError);
  if (bError) { return; }

  emit spSignalEmitter->showMedia(sResource);
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::play()
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CMediaPlayerSignalEmitter>()->playMedia(QString());
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::play(QJSValue resource)
{
  if (!CheckIfScriptCanRun()) { return; }

  auto pSignalEmitter = SignalEmitter<CMediaPlayerSignalEmitter>();
  bool bError = false;
  QString sResource = GetResourceName(resource, &bError);
  if (bError) { return; }

  emit pSignalEmitter->playMedia(sResource);
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
void CScriptMediaPlayer::pauseVideo()
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CMediaPlayerSignalEmitter>()->pauseVideo();
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
void CScriptMediaPlayer::playSound(QJSValue resource)
{
  if (!CheckIfScriptCanRun()) { return; }

  bool bError = false;
  QString sResource = GetResourceName(resource, &bError);
  if (bError) { return; }

  emit SignalEmitter<CMediaPlayerSignalEmitter>()->playSound(sResource);
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::pauseSound(QJSValue resource)
{
  if (!CheckIfScriptCanRun()) { return; }

  bool bError = false;
  QString sResource = GetResourceName(resource, &bError);
  if (bError) { return; }

  emit SignalEmitter<CMediaPlayerSignalEmitter>()->pauseSound(sResource);
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::stopSound(QJSValue resource)
{
  if (!CheckIfScriptCanRun()) { return; }

  bool bError = false;
  QString sResource = GetResourceName(resource, &bError);
  if (bError) { return; }

  emit SignalEmitter<CMediaPlayerSignalEmitter>()->stopSound(sResource);
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
void CScriptMediaPlayer::waitForPlayback(QJSValue resource)
{
  if (!CheckIfScriptCanRun()) { return; }

  bool bError = false;
  QString sResource = GetResourceName(resource, &bError);
  if (bError) { return; }

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
  emit pSignalEmitter->startVideoWait();
  loop.exec();

  disconnect(pSignalEmitter, &CMediaPlayerSignalEmitter::videoFinished,
             &loop, &QEventLoop::quit);
  disconnect(pSignalEmitter, &CMediaPlayerSignalEmitter::interrupt,
             &loop, &QEventLoop::quit);
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
void CScriptMediaPlayer::waitForSound(QJSValue resource)
{
  if (!CheckIfScriptCanRun()) { return; }

  bool bError = false;
  QString sResource = GetResourceName(resource, &bError);
  if (bError) { return; }

  WaitForSoundImpl(sResource);
}

//----------------------------------------------------------------------------------------
//
QString CScriptMediaPlayer::GetResourceName(const QJSValue& resource, bool* pbError)
{
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != pbError) { *pbError = false; }
  QString sResource;
  if (nullptr != spDbManager)
  {
    if (resource.isString())
    {
      sResource = resource.toString();
      tspResource spResource = spDbManager->FindResourceInProject(m_spProject, sResource);
      if (nullptr == spResource)
      {
        QString sError = tr("Resource %1 not found");
        emit m_pSignalEmitter->showError(sError.arg(resource.toString()),
                                          QtMsgType::QtWarningMsg);
        if (nullptr != pbError) { *pbError = true; }
      }
    }
    else if (resource.isQObject())
    {
      CResource* pResource = dynamic_cast<CResource*>(resource.toQObject());
      if (nullptr != pResource)
      {
        if (nullptr != pResource->Data())
        {
          sResource = pResource->getName();
        }
        else
        {
          QString sError = tr("Resource in play() holds no data.");
          emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
          if (nullptr != pbError) { *pbError = true; }
        }
      }
      else
      {
        QString sError = tr("Wrong argument-type to play(). String, resource or null was expected.");
        emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
        if (nullptr != pbError) { *pbError = true; }
      }
    }
    else
    {
      QString sError = tr("Wrong argument-type to play(). String, resource or null was expected.");
      emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
      if (nullptr != pbError) { *pbError = true; }
    }
  }

  return sResource;
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
  emit pSignalEmitter->startPlaybackWait(sResource);
  loop.exec();

  disconnect(connQuit);
  disconnect(pSignalEmitter, &CMediaPlayerSignalEmitter::interrupt,
             &loop, &QEventLoop::quit);
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
  emit pSignalEmitter->startSoundWait(sResource);
  loop.exec();

  disconnect(connQuit);
  disconnect(pSignalEmitter, &CMediaPlayerSignalEmitter::interrupt,
             &loop, &QEventLoop::quit);
  loop.disconnect();

  disconnect(connStop);
}
