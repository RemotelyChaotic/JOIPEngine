#include "ScriptMediaPlayer.h"
#include "Application.h"
#include "DatabaseManager.h"
#include "ScriptRunnerSignalEmiter.h"

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
void CScriptMediaPlayer::play(QJSValue resource)
{
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
  emit m_spSignalEmitter->SignalPauseVideo();
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::stopVideo()
{
  emit m_spSignalEmitter->SignalStopVideo();
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::pauseSound()
{
  emit m_spSignalEmitter->SignalPauseSound();
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::stopSound()
{
  emit m_spSignalEmitter->SignalStopSound();
}
