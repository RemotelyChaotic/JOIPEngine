#include "ScriptMediaPlayer.h"
#include "Application.h"
#include "DatabaseManager.h"
#include "ScriptRunnerSignalEmiter.h"
#include <QTimer>

CScriptMediaPlayer::CScriptMediaPlayer(std::shared_ptr<CScriptRunnerSignalEmiter> spEmitter,
                                       QJSEngine* pEngine) :
  QObject(),
  m_spSignalEmitter(spEmitter),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_pEngine(pEngine)
{
}

CScriptMediaPlayer::~CScriptMediaPlayer()
{
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::SetCurrentProject(tspProject spProject)
{
  m_spProject = spProject;
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::show(QJSValue resource)
{
  if (!CheckIfScriptCanRun()) { return; }

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    if (resource.isString())
    {
      tspResource spResource = spDbManager->FindResource(m_spProject, resource.toString());
      if (nullptr != spResource)
      {
        emit m_spSignalEmitter->SignalShowMedia(spResource);
      }
      else
      {
        QString sError = tr("Resource %1 not found");
        emit m_spSignalEmitter->SignalShowError(sError.arg(resource.toString()),
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
          emit m_spSignalEmitter->SignalShowMedia(spResource);
        }
        else
        {
          QString sError = tr("Resource in show() holds no data.");
          emit m_spSignalEmitter->SignalShowError(sError, QtMsgType::QtWarningMsg);
        }
      }
      else
      {
        QString sError = tr("Wrong argument-type to show(). String or resource was expected.");
        emit m_spSignalEmitter->SignalShowError(sError, QtMsgType::QtWarningMsg);
      }
    }
    else
    {
      QString sError = tr("Wrong argument-type to show(). String or resource was expected.");
      emit m_spSignalEmitter->SignalShowError(sError, QtMsgType::QtWarningMsg);
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

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    if (resource.isString())
    {
      tspResource spResource = spDbManager->FindResource(m_spProject, resource.toString());
      if (nullptr != spResource)
      {
        emit m_spSignalEmitter->SignalPlayMedia(spResource);
      }
      else
      {
        QString sError = tr("Resource %1 not found");
        emit m_spSignalEmitter->SignalShowError(sError.arg(resource.toString()),
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
          emit m_spSignalEmitter->SignalPlayMedia(spResource);
        }
        else
        {
          QString sError = tr("Resource in play() holds no data.");
          emit m_spSignalEmitter->SignalShowError(sError, QtMsgType::QtWarningMsg);
        }
      }
      else
      {
        QString sError = tr("Wrong argument-type to play(). String or resource was expected.");
        emit m_spSignalEmitter->SignalShowError(sError, QtMsgType::QtWarningMsg);
      }
    }
    else if (resource.isNull())
    {
      emit m_spSignalEmitter->SignalPlayMedia(nullptr);
    }
    else
    {
      QString sError = tr("Wrong argument-type to play(). String or resource was expected.");
      emit m_spSignalEmitter->SignalShowError(sError, QtMsgType::QtWarningMsg);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::pauseVideo()
{
  if (!CheckIfScriptCanRun()) { return; }
  emit m_spSignalEmitter->SignalPauseVideo();
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::stopVideo()
{
  if (!CheckIfScriptCanRun()) { return; }
  emit m_spSignalEmitter->SignalStopVideo();
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::pauseSound()
{
  if (!CheckIfScriptCanRun()) { return; }
  emit m_spSignalEmitter->SignalPauseSound();
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::stopSound()
{
  if (!CheckIfScriptCanRun()) { return; }
  emit m_spSignalEmitter->SignalStopSound();
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::waitForPlayback()
{
  if (!CheckIfScriptCanRun()) { return; }

  connect(m_spSignalEmitter.get(), &CScriptRunnerSignalEmiter::SignalPlaybackFinished,
          m_spSignalEmitter.get(), &CScriptRunnerSignalEmiter::SignalStopVideo, Qt::DirectConnection);

  QEventLoop loop;
  connect(m_spSignalEmitter.get(), &CScriptRunnerSignalEmiter::SignalPlaybackFinished,
          &loop, &QEventLoop::quit, Qt::QueuedConnection);
  connect(m_spSignalEmitter.get(), &CScriptRunnerSignalEmiter::SignalInterruptLoops,
          &loop, &QEventLoop::quit, Qt::QueuedConnection);
  loop.exec();
  loop.disconnect();

  disconnect(m_spSignalEmitter.get(), &CScriptRunnerSignalEmiter::SignalPlaybackFinished,
             m_spSignalEmitter.get(), &CScriptRunnerSignalEmiter::SignalStopVideo);
}

//----------------------------------------------------------------------------------------
//
bool CScriptMediaPlayer::CheckIfScriptCanRun()
{
  if (m_spSignalEmitter->ScriptExecutionStatus()._to_integral() == EScriptExecutionStatus::eStopped)
  {
    QJSValue val = m_pEngine->evaluate("f();"); //undefined function -> create error
    Q_UNUSED(val);
    return false;
  }
  else
  {
    return true;
  }
}
