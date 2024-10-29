#include "MultiEmitterSoundPlayer.h"
#include <QtAV>
#include <QFileInfo>

const qint32 CMultiEmitterSoundPlayer::c_iDefaultNumAutioEmitters = 5;

CMultiEmitterSoundPlayer::CMultiEmitterSoundPlayer(qint32 iNrSoundEmitters) :
  m_vspPlayers(),
  m_iNrSoundEmitters(iNrSoundEmitters),
  m_iCurrentAutioPlayer(-1),
  m_iNextSfxToLoad(0),
  m_bMuted(false)
{
}
CMultiEmitterSoundPlayer::CMultiEmitterSoundPlayer(qint32 iNrSoundEmitters,
                                                   const QString& sSoundEffect) :
  CMultiEmitterSoundPlayer(iNrSoundEmitters)
{
  m_sSoundEffects = QStringList{} << sSoundEffect;
  Initialize();
}
CMultiEmitterSoundPlayer::CMultiEmitterSoundPlayer(qint32 iNrSoundEmitters,
                                                   const QStringList& sSoundEffects) :
  CMultiEmitterSoundPlayer(iNrSoundEmitters)
{
  m_sSoundEffects = sSoundEffects;
  Initialize();
}
CMultiEmitterSoundPlayer::~CMultiEmitterSoundPlayer()
{

}

//----------------------------------------------------------------------------------------
//
const QStringList& CMultiEmitterSoundPlayer::SoundEffects() const
{
  return m_sSoundEffects;
}

//----------------------------------------------------------------------------------------
//
void CMultiEmitterSoundPlayer::SetSoundEffect(const QString& sPath)
{
  QStringList vsNew = QStringList() << sPath;
  SetSoundEffects(vsNew);
}

//----------------------------------------------------------------------------------------
//
void CMultiEmitterSoundPlayer::SetSoundEffects(const QStringList& vsNew)
{
  if (m_sSoundEffects != vsNew && !vsNew.isEmpty())
  {
    if (!vsNew.isEmpty())
    {
      m_sSoundEffects = vsNew;
      for (qint32 i = 0; static_cast<qint32>(m_vspPlayers.size()) > i; ++i)
      {
        m_vspPlayers[i]->setFile(m_sSoundEffects[i%m_sSoundEffects.size()]);
        m_vspPlayers[i]->load();
        AdvanceNextSfxToLoad();
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
    // next audio player
    qint32 iLastPlayer = m_iCurrentAutioPlayer;
    if (static_cast<qint32>(m_vspPlayers.size()) <= ++m_iCurrentAutioPlayer)
    {
      m_iCurrentAutioPlayer = 0;
    }

    // play sound
    if (-1 < m_iCurrentAutioPlayer && static_cast<qint32>(m_vspPlayers.size()) > m_iCurrentAutioPlayer)
    {
      m_vspPlayers[m_iCurrentAutioPlayer]->stop();
      m_vspPlayers[m_iCurrentAutioPlayer]->play();
    }

    // cycle sound files
    if (-1 < iLastPlayer && static_cast<qint32>(m_vspPlayers.size()) > iLastPlayer)
    {
      if (m_vspPlayers[iLastPlayer]->file() != m_sSoundEffects[m_iNextSfxToLoad])
      {
        m_vspPlayers[iLastPlayer]->setFile(m_sSoundEffects[m_iNextSfxToLoad]);
        m_vspPlayers[iLastPlayer]->load();
      }
      AdvanceNextSfxToLoad();
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

//----------------------------------------------------------------------------------------
//
void CMultiEmitterSoundPlayer::AdvanceNextSfxToLoad()
{
  ++m_iNextSfxToLoad;
  if (m_sSoundEffects.size() <= m_iNextSfxToLoad)
  {
    m_iNextSfxToLoad = 0;
  }
}

//----------------------------------------------------------------------------------------
//
void CMultiEmitterSoundPlayer::Initialize()
{
  for (qint32 i = 0; m_iNrSoundEmitters > i; ++i)
  {
    m_vspPlayers.push_back(std::make_unique<QtAV::AVPlayer>());
    m_vspPlayers.back()->setRepeat(0);
    m_vspPlayers.back()->setFile(m_sSoundEffects[i%m_sSoundEffects.size()]);
    m_vspPlayers.back()->load();
    AdvanceNextSfxToLoad();
  }
}
