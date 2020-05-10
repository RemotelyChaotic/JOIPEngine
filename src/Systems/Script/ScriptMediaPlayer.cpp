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
      tspResource spResource = spDbManager->FindResourceInProject(m_spProject, resource.toString());
      if (nullptr != spResource)
      {
        emit spSignalEmitter->showMedia(spResource);
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
        tspResource spResource = pResource->Data();
        if (nullptr != spResource)
        {
          emit spSignalEmitter->showMedia(spResource);
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
      tspResource spResource = spDbManager->FindResourceInProject(m_spProject, resource.toString());
      if (nullptr != spResource)
      {
        emit pSignalEmitter->playMedia(spResource);
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
        tspResource spResource = pResource->Data();
        if (nullptr != spResource)
        {
          emit pSignalEmitter->playMedia(spResource);
        }
        else
        {
          QString sError = tr("Resource in play() holds no data.");
          emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
        }
      }
      else
      {
        QString sError = tr("Wrong argument-type to play(). String or resource was expected.");
        emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
      }
    }
    else if (resource.isNull())
    {
      emit pSignalEmitter->playMedia(nullptr);
    }
    else
    {
      QString sError = tr("Wrong argument-type to play(). String or resource was expected.");
      emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
    }
  }
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

  QEventLoop loop;
  connect(pSignalEmitter, &CMediaPlayerSignalEmitter::playbackFinished,
          &loop, &QEventLoop::quit, Qt::QueuedConnection);
  connect(pSignalEmitter, &CMediaPlayerSignalEmitter::interrupt,
          &loop, &QEventLoop::quit, Qt::QueuedConnection);
  loop.exec();
  loop.disconnect();

  disconnect(pSignalEmitter, &CMediaPlayerSignalEmitter::playbackFinished,
             pSignalEmitter, &CMediaPlayerSignalEmitter::stopVideo);
}
