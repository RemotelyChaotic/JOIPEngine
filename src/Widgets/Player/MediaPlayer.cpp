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
#include "AvSlider.h"

#include <QPushButton>
#include <QSlider>
#include <QLayout>
#include <QLabel>
#include <QMessageBox>
#include <QFileDialog>
#include <QToolTip>

#include <QtAVWidgets>

#include <limits>

using namespace QtAV;

CMediaPlayer::CMediaPlayer(QWidget* pParent) :
  QWidget(pParent)
{
  m_unit = 1000;
  QVBoxLayout *vl = new QVBoxLayout();
  vl->setSpacing(0);
  setLayout(vl);
  Load();
}

CMediaPlayer::~CMediaPlayer()
{
}

//----------------------------------------------------------------------------------------
//
bool CMediaPlayer::IsMuted()
{
  if (!m_bLoaded) { return false; }
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
  if (!m_bLoaded) { return false; }
  return m_player->isPlaying();
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::SetSliderVisible(bool bVisible)
{
  if (!m_bLoaded) { return; }
  m_slider->setVisible(bVisible);
  m_timingInfo->setVisible(bVisible);
}

//----------------------------------------------------------------------------------------
//
double CMediaPlayer::Volume()
{
  if (!m_bLoaded) { return 0.0; }
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
  if (!m_bLoaded) { return; }
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
  if (!m_bLoaded) { return; }
  if (sPath.isEmpty()) { return; }
  m_player->setStartPosition(0);
  m_player->setStopPosition();
  m_player->setRepeat(std::numeric_limits<int>::max());
  m_player->play(sPath);
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::SeekBySlider(qint32 value)
{
  if (!m_bLoaded) { return; }
  if (!m_player->isPlaying())  { return; }
  qDebug("seekbyslider: %d", value);
  m_player->seek(qint64(value*m_unit));
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::SeekBySlider()
{
  if (!m_bLoaded) { return; }
  qDebug("pressed: %d", m_slider->value());
  SeekBySlider(m_slider->value());
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::PlayPause()
{
  if (!m_bLoaded) { return; }
  if (!m_player->isPlaying())
  {
    m_player->setStartPosition(0);
    m_player->setStopPosition();
    m_player->setRepeat(std::numeric_limits<int>::max());
    m_player->play();
    return;
  }
  m_player->pause(!m_player->isPaused());
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::Unload()
{
  if (m_bLoaded)
  {
    QVBoxLayout* pLayout = dynamic_cast<QVBoxLayout*>(layout());
    if (nullptr != pLayout)
    {
      while (QLayoutItem* pItem = pLayout->takeAt(0))
      {
        if (nullptr != pItem) { delete pItem; }
        if (nullptr != pItem->widget() && m_vo->widget() != pItem->widget()) { delete pItem->widget(); }
      }
    }

    m_player->setRenderer(nullptr);
    delete m_player;
    delete m_vo;
    if (m_preview)
    {
      m_preview->close();
      delete m_preview;
    }

    m_vo = nullptr;
    m_player = nullptr;
    m_bLoaded = false;
  }
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::SetVolume(double dVolume)
{
  if (!m_bLoaded) { return; }
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
  if (!m_bLoaded) { return; }
  m_player->setRepeat(0);
  m_player->stop();
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::OnTimeSliderHover(qint32 pos, qint32 value)
{
  QPoint gpos = m_slider->parentWidget()->mapToGlobal(m_slider->pos() + QPoint(pos, 0));
  QToolTip::showText(gpos, QTime(0, 0, 0).addMSecs(value*m_unit).toString("mm:ss"));
  if (!m_preview)
  {
    m_preview = new VideoPreviewWidget();
  }
  m_preview->setFile(m_player->file());
  m_preview->setTimestamp(value);
  m_preview->preview();
  static const int w = 100;
  static const int h = 100;
  m_preview->setWindowFlags(Qt::Tool |Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
  m_preview->resize(w, h);
  m_preview->move(gpos - QPoint(w/2, h));
  m_preview->show();
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::OnTimeSliderLeave()
{
  if (!m_preview)
  {
    return;
  }
  if (m_preview->isVisible())
  {
    m_preview->close();
  }
  delete m_preview;
  m_preview = nullptr;
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::UpdateSlider(qint64 value)
{
  if (!m_bLoaded) { return; }
  m_slider->blockSignals(true);
  m_slider->setRange(0, int(m_player->duration()/m_unit));
  m_slider->setValue(int(value/m_unit));
  m_timingInfo->setText(QTime(0, 0, 0).addMSecs(value).toString("mm:ss") + " / " +
                        QTime(0, 0, 0).addMSecs(m_player->mediaStopPosition()).toString("mm:ss"));
  m_slider->blockSignals(false);
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::UpdateSlider()
{
  if (!m_bLoaded) { return; }
  UpdateSlider(m_player->position());
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::UpdateSliderUnit()
{
  if (!m_bLoaded) { return; }
  m_unit = m_player->notifyInterval();
  UpdateSlider();
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::Load()
{
  if (!m_bLoaded)
  {
    QVBoxLayout* vl = dynamic_cast<QVBoxLayout*>(layout());
    if (nullptr == vl)
    {
      qWarning() << "No layout in player";
      return;
    }

    m_player = new AVPlayer(this);
    m_vo = new VideoOutput(this);
    if (!m_vo->widget())
    {
        qWarning() << tr("Can not create video renderer");
        return;
    }
    m_vo->setBackgroundColor(Qt::transparent);
    m_vo->widget()->setAttribute(Qt::WA_TranslucentBackground);
    m_vo->widget()->setAttribute(Qt::WA_AlwaysStackOnTop);
    m_vo->widget()->setWindowFlag(Qt::FramelessWindowHint);
    m_player->setRenderer(m_vo);

    QWidget* pWidget = new QWidget(this);
    pWidget->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
    QVBoxLayout* vl2 = new QVBoxLayout();
    vl2->setSpacing(0);
    pWidget->setLayout(vl2);

    m_slider = new CAvSlider(pWidget);
    m_slider->setOrientation(Qt::Horizontal);
    m_slider->setTracking(true);
    m_timingInfo = new QLabel("00:00 / 00:00", pWidget);
    m_timingInfo->setAlignment(Qt::AlignHCenter);
    connect(m_slider, &QSlider::valueChanged,
            this, static_cast<void (CMediaPlayer::*)(qint32)>(&CMediaPlayer::SeekBySlider));
    connect(m_slider, &CAvSlider::onHover, this, &CMediaPlayer::OnTimeSliderHover);
    connect(m_slider, &CAvSlider::onLeave, this, &CMediaPlayer::OnTimeSliderLeave);
    connect(m_player, &AVPlayer::positionChanged,
            this, static_cast<void (CMediaPlayer::*)(qint64)>(&CMediaPlayer::UpdateSlider));
    connect(m_player, &AVPlayer::started,
            this, static_cast<void (CMediaPlayer::*)(void)>(&CMediaPlayer::UpdateSlider));
    connect(m_player, &AVPlayer::notifyIntervalChanged, this, &CMediaPlayer::UpdateSliderUnit);
    connect(m_player, &AVPlayer::mediaStatusChanged, this, &CMediaPlayer::MediaStatusChanged);

    vl2->addWidget(m_slider);
    vl2->addWidget(m_timingInfo);

    vl->addWidget(m_vo->widget());
    vl->addWidget(pWidget);

    m_bLoaded = true;
  }
}
