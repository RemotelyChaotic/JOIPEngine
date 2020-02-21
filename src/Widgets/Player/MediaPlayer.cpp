/******************************************************************************
    Simple Player:  this file is part of QtAV examples
    Copyright (C) 2012-2016 Wang Bin <wbsecg1@gmail.com>

*   This file is part of QtAV

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include "MediaPlayer.h"
#include <QPushButton>
#include <QSlider>
#include <QLayout>
#include <QMessageBox>
#include <QFileDialog>

using namespace QtAV;

CMediaPlayer::CMediaPlayer(QWidget* pParent) :
  QWidget(pParent)
{
  m_unit = 1000;
  m_player = new AVPlayer(this);
  QVBoxLayout *vl = new QVBoxLayout();
  setLayout(vl);
  m_vo = new VideoOutput(this);
  if (!m_vo->widget()) {
      qWarning() << tr("Can not create video renderer");
      return;
  }
  m_vo->setBackgroundColor(Qt::transparent);
  m_vo->widget()->setAttribute(Qt::WA_TranslucentBackground);
  m_vo->widget()->setAttribute(Qt::WA_AlwaysStackOnTop);
  m_vo->widget()->setWindowFlag(Qt::FramelessWindowHint);
  m_player->setRenderer(m_vo);
  vl->addWidget(m_vo->widget());
  m_slider = new QSlider();
  m_slider->setOrientation(Qt::Horizontal);
  connect(m_slider, &QSlider::sliderMoved,
          this, static_cast<void (CMediaPlayer::*)(qint32)>(&CMediaPlayer::SeekBySlider));
  connect(m_slider, &QSlider::sliderPressed,
          this, static_cast<void (CMediaPlayer::*)(void)>(&CMediaPlayer::SeekBySlider));
  connect(m_player, &AVPlayer::positionChanged,
          this, static_cast<void (CMediaPlayer::*)(qint64)>(&CMediaPlayer::UpdateSlider));
  connect(m_player, &AVPlayer::started,
          this, static_cast<void (CMediaPlayer::*)(void)>(&CMediaPlayer::UpdateSlider));
  connect(m_player, &AVPlayer::notifyIntervalChanged,
          this, &CMediaPlayer::UpdateSliderUnit);

  vl->addWidget(m_slider);
  QHBoxLayout *hb = new QHBoxLayout();
  vl->addLayout(hb);
}

CMediaPlayer::~CMediaPlayer()
{
}

//----------------------------------------------------------------------------------------
//
bool CMediaPlayer::IsMuted()
{
  auto pAudio = m_player->audio();
  if (nullptr != pAudio)
  {
    return pAudio->isMute();
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
bool CMediaPlayer::IsPlaying()
{
  return m_player->isPlaying();
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::SetSliderVisible(bool bVisible)
{
  m_slider->setVisible(bVisible);
}

//----------------------------------------------------------------------------------------
//
double CMediaPlayer::Volume()
{
  auto pAudio = m_player->audio();
  if (nullptr != pAudio)
  {
    return pAudio->volume();
  }
  return 0.0;
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::MuteUnmute(bool bMuted)
{
  auto pAudio = m_player->audio();
  if (nullptr != pAudio)
  {
    pAudio->setMute(bMuted);
  }
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::OpenMedia(const QString sPath)
{
  if (sPath.isEmpty()) { return; }
  m_player->play(sPath);
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::SeekBySlider(qint32 value)
{
  if (!m_player->isPlaying())  { return; }
  qDebug("seekbyslider: %d", value);
  m_player->seek(qint64(value*m_unit));
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::SeekBySlider()
{
  qDebug("pressed: %d", m_slider->value());
  SeekBySlider(m_slider->value());
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::PlayPause()
{
  if (!m_player->isPlaying())
  {
    m_player->play();
    return;
  }
  m_player->pause(!m_player->isPaused());
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::SetVolume(double dVolume)
{
  auto pAudio = m_player->audio();
  if (nullptr != pAudio)
  {
    pAudio->setVolume(dVolume);
  }
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::Stop()
{
  if (m_player->isPlaying())
  {
    m_player->stop();
  }
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::UpdateSlider(qint64 value)
{
  m_slider->setRange(0, int(m_player->duration()/m_unit));
  m_slider->setValue(int(value/m_unit));
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::UpdateSlider()
{
  UpdateSlider(m_player->position());
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::UpdateSliderUnit()
{
  m_unit = m_player->notifyInterval();
  UpdateSlider();
}
