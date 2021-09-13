#include "MultiEmitterSoundPlayer.h"
#include <QtAV>
#include <QFileInfo>

const qint32 CMultiEmitterSoundPlayer::c_iDefaultNumAutioEmitters = 5;

CMultiEmitterSoundPlayer::CMultiEmitterSoundPlayer(qint32 iNrSoundEmitters,
                                                   const QString& sSoundEffect) :
  m_vspPlayers(),
  m_iNrSoundEmitters(iNrSoundEmitters),
  m_sSoundEffect(sSoundEffect),
  m_iLastAutioPlayer(0),
  m_bMuted(false)
{
  for (qint32 i = 0; m_iNrSoundEmitters > i; ++i)
  {
    m_vspPlayers.push_back(std::make_unique<QtAV::AVPlayer>());
    m_vspPlayers.back()->setRepeat(0);
    m_vspPlayers.back()->setFile(m_sSoundEffect);
  }
}
CMultiEmitterSoundPlayer::~CMultiEmitterSoundPlayer()
{

}

//----------------------------------------------------------------------------------------
//
const QString& CMultiEmitterSoundPlayer::SoundEffect() const
{
  return m_sSoundEffect;
}

//----------------------------------------------------------------------------------------
//
void CMultiEmitterSoundPlayer::SetSoundEffect(const QString& sPath)
{
  if (m_sSoundEffect != sPath && (sPath.isEmpty() || QFileInfo(sPath).exists()))
  {
    if (!sPath.isEmpty())
    {
      m_sSoundEffect = sPath;
      for (qint32 i = 0; static_cast<qint32>(m_vspPlayers.size()) > i; ++i)
      {
        m_vspPlayers[i]->setFile(m_sSoundEffect);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
bool CMultiEmitterSoundPlayer::Muted() const
{
  return m_bMuted;
}

//----------------------------------------------------------------------------------------
//
void CMultiEmitterSoundPlayer::SetMuted(bool bMuted)
{
  if (m_bMuted != bMuted)
  {
    m_bMuted = bMuted;
    if (m_bMuted)
    {
      for (qint32 i = 0; static_cast<qint32>(m_vspPlayers.size()) > i; ++i)
      {
        if (m_vspPlayers[i]->isPlaying())
        {
          m_vspPlayers[i]->stop();
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
double CMultiEmitterSoundPlayer::Volume() const
{
  return m_dVolume;
}

//----------------------------------------------------------------------------------------
//
void CMultiEmitterSoundPlayer::SetVolume(double dValue)
{
  for (qint32 i = 0; static_cast<qint32>(m_vspPlayers.size()) > i; ++i)
  {
    if (nullptr != m_vspPlayers[i]->audio())
    {
      m_vspPlayers[i]->audio()->setVolume(dValue);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CMultiEmitterSoundPlayer::Play()
{
  if (!m_bMuted)
  {
    // play sound
    if (-1 < m_iLastAutioPlayer && static_cast<qint32>(m_vspPlayers.size()) > m_iLastAutioPlayer)
    {
      if (m_vspPlayers[m_iLastAutioPlayer]->isPlaying())
      {
        m_vspPlayers[m_iLastAutioPlayer]->stop();
      }
      m_vspPlayers[m_iLastAutioPlayer]->play();
    }

    // next audio player
    if (static_cast<qint32>(m_vspPlayers.size()) <= ++m_iLastAutioPlayer)
    {
      m_iLastAutioPlayer = 0;
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CMultiEmitterSoundPlayer::Stop()
{
  for (qint32 i = 0; static_cast<qint32>(m_vspPlayers.size()) > i; ++i)
  {
    if (m_vspPlayers[i]->isPlaying())
    {
      m_vspPlayers[i]->stop();
    }
  }
}
