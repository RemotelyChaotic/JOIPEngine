#include "ScriptMediaPlayer.h"

CScriptMediaPlayer::CScriptMediaPlayer(QJSEngine* pEngine) :
  QObject(),
  m_pEngine(pEngine)
{
}

CScriptMediaPlayer::~CScriptMediaPlayer()
{
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::show(QJSValue resource)
{
  Q_UNUSED(resource);
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::play(QJSValue resource)
{
  Q_UNUSED(resource);
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::playVideo(QJSValue resource)
{
  Q_UNUSED(resource);
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::pauseVideo()
{

}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::stopVideo()
{

}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::playSound(QJSValue resource)
{
  Q_UNUSED(resource);
}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::pauseSound()
{

}

//----------------------------------------------------------------------------------------
//
void CScriptMediaPlayer::stopSound()
{

}
