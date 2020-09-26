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
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    if (resource.isString())
    {
      QString sResource = resource.toString();
      tspResource spResource = spDbManager->FindResourceInProject(m_spProject, sResource);
      if (nullptr != spResource)
      {
        emit spSignalEmitter->showMedia(sResource);
      }
      else
      {
        QString sError = tr("Resource %1 not found");
        emit m_pSignalEmitter->showError(sError.arg(resource.toString()),
                                                QtMsgType::QtWarningMsg);
      }
    }
    else if (resource.isQObject())
    {
      CResource* pResource = dynamic_cast<CResource*>(resource.toQObject());
      if (nullptr != pResource)
      {
        if (nullptr != pResource->Data())
        {
          emit spSignalEmitter->showMedia(pResource->getName());
        }
        else
        {
          QString sError = tr("Resource in show() holds no data.");
          emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
        }
      }
      else
      {
        QString sError = tr("Wrong argument-type to show(). String or resource was expected.");
        emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
      }
    }
    else if (resource.isNull())
    {
      emit spSignalEmitter->showMedia(QString());
    }
    else
    {
      QString sError = tr("Wrong argument-type to show(). String or resource was expected.");
      emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::play()
{
  play(QJSValue::NullValue);
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::play(QJSValue resource)
{
  if (!CheckIfScriptCanRun()) { return; }

  auto pSignalEmitter = SignalEmitter<CMediaPlayerSignalEmitter>();
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    if (resource.isString())
    {
      QString sResource = resource.toString();
      tspResource spResource = spDbManager->FindResourceInProject(m_spProject, sResource);
      if (nullptr != spResource)
      {
        emit pSignalEmitter->playMedia(sResource);
      }
      else
      {
        QString sError = tr("Resource %1 not found");
        emit m_pSignalEmitter->showError(sError.arg(resource.toString()),
                                          QtMsgType::QtWarningMsg);
      }
    }
    else if (resource.isQObject())
    {
      CResource* pResource = dynamic_cast<CResource*>(resource.toQObject());
      if (nullptr != pResource)
      {
        if (nullptr != pResource->Data())
        {
          emit pSignalEmitter->playMedia(pResource->getName());
        }
        else
        {
          QString sError = tr("Resource in play() holds no data.");
          emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
        }
      }
      else
      {
        QString sError = tr("Wrong argument-type to play(). String, resource or null was expected.");
        emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
      }
    }
    else if (resource.isNull())
    {
      emit pSignalEmitter->playMedia(QString());
    }
    else
    {
      QString sError = tr("Wrong argument-type to play(). String, resource or null was expected.");
      emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
    }
  }
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
void CScriptMediaPlayer::playSound()
{
  if (!CheckIfScriptCanRun()) { return; }

  emit SignalEmitter<CMediaPlayerSignalEmitter>()->playSound();
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::pauseSound()
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CMediaPlayerSignalEmitter>()->pauseSound();
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::stopSound()
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CMediaPlayerSignalEmitter>()->stopSound();
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::waitForPlayback()
{
  if (!CheckIfScriptCanRun()) { return; }

  auto pSignalEmitter = SignalEmitter<CMediaPlayerSignalEmitter>();

  connect(pSignalEmitter, &CMediaPlayerSignalEmitter::playbackFinished,
          pSignalEmitter, &CMediaPlayerSignalEmitter::stopVideo, Qt::DirectConnection);
  connect(pSignalEmitter, &CMediaPlayerSignalEmitter::playbackFinished,
          pSignalEmitter, &CMediaPlayerSignalEmitter::stopSound, Qt::DirectConnection);

  QEventLoop loop;
  connect(pSignalEmitter, &CMediaPlayerSignalEmitter::playbackFinished,
          &loop, &QEventLoop::quit, Qt::QueuedConnection);
  connect(pSignalEmitter, &CMediaPlayerSignalEmitter::interrupt,
          &loop, &QEventLoop::quit, Qt::QueuedConnection);
  emit pSignalEmitter->startPlaybackWait();
  loop.exec();

  disconnect(pSignalEmitter, &CMediaPlayerSignalEmitter::playbackFinished,
             &loop, &QEventLoop::quit);
  disconnect(pSignalEmitter, &CMediaPlayerSignalEmitter::interrupt,
             &loop, &QEventLoop::quit);
  loop.disconnect();

  disconnect(pSignalEmitter, &CMediaPlayerSignalEmitter::playbackFinished,
             pSignalEmitter, &CMediaPlayerSignalEmitter::stopVideo);
  disconnect(pSignalEmitter, &CMediaPlayerSignalEmitter::playbackFinished,
             pSignalEmitter, &CMediaPlayerSignalEmitter::stopSound);
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

  auto pSignalEmitter = SignalEmitter<CMediaPlayerSignalEmitter>();

  connect(pSignalEmitter, &CMediaPlayerSignalEmitter::soundFinished,
          pSignalEmitter, &CMediaPlayerSignalEmitter::stopSound, Qt::DirectConnection);

  QEventLoop loop;
  connect(pSignalEmitter, &CMediaPlayerSignalEmitter::soundFinished,
          &loop, &QEventLoop::quit, Qt::QueuedConnection);
  connect(pSignalEmitter, &CMediaPlayerSignalEmitter::interrupt,
          &loop, &QEventLoop::quit, Qt::QueuedConnection);
  emit pSignalEmitter->startSoundWait();
  loop.exec();

  disconnect(pSignalEmitter, &CMediaPlayerSignalEmitter::soundFinished,
             &loop, &QEventLoop::quit);
  disconnect(pSignalEmitter, &CMediaPlayerSignalEmitter::interrupt,
             &loop, &QEventLoop::quit);
  loop.disconnect();

  disconnect(pSignalEmitter, &CMediaPlayerSignalEmitter::soundFinished,
             pSignalEmitter, &CMediaPlayerSignalEmitter::stopSound);
}
