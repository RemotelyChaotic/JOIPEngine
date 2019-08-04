/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "MediaPlayer.h"

#include "MediaPlayerControls.h"
#include "MediaPlayerHistogramWidget.h"
#include "MediaPlayerVideoWidget.h"

#include <QMediaService>
#include <QMediaPlaylist>
#include <QVideoProbe>
#include <QAudioProbe>
#include <QMediaMetaData>
#include <QtWidgets>

CMediaPlayer::CMediaPlayer(QWidget *parent)
  : QWidget(parent)
{
  m_player = new QMediaPlayer(this);
  m_player->setAudioRole(QAudio::VideoRole);
  qDebug() << "Supported audio roles:";
  for (QAudio::Role role : m_player->supportedAudioRoles())
    qDebug() << "    " << role;
  // owned by PlaylistModel
  m_playlist = new QMediaPlaylist();
  m_player->setPlaylist(m_playlist);

  connect(m_player, &QMediaPlayer::durationChanged, this, &CMediaPlayer::SlotDrationChanged);
  connect(m_player, &QMediaPlayer::positionChanged, this, &CMediaPlayer::SlotPositionChanged);
  connect(m_player, QOverload<>::of(&QMediaPlayer::metaDataChanged), this, &CMediaPlayer::SlotMetaDataChanged);
  connect(m_playlist, &QMediaPlaylist::currentIndexChanged, this, &CMediaPlayer::SlotPlaylistPositionChanged);
  connect(m_player, &QMediaPlayer::mediaStatusChanged, this, &CMediaPlayer::SlotStatusChanged);
  connect(m_player, &QMediaPlayer::bufferStatusChanged, this, &CMediaPlayer::SlotBufferingProgress);
  connect(m_player, &QMediaPlayer::videoAvailableChanged, this, &CMediaPlayer::SlotVideoAvailableChanged);
  connect(m_player, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error), this, &CMediaPlayer::SlotDisplayErrorMessage);
  connect(m_player, &QMediaPlayer::stateChanged, this, &CMediaPlayer::SlotStateChanged);

  m_videoWidget = new CMediaPlayerVideoWidget(this);
  m_player->setVideoOutput(m_videoWidget);

  m_slider = new QSlider(Qt::Horizontal, this);
  m_slider->setRange(0, static_cast<qint32>(m_player->duration() / 1000));

  m_labelDuration = new QLabel(this);
  connect(m_slider, &QSlider::sliderMoved, this, &CMediaPlayer::SlotSeek);

  m_labelHistogram = new QLabel(this);
  m_labelHistogram->setText("Histogram:");
  m_videoHistogram = new CMediaPlayerHistogramWidget(this);
  m_audioHistogram = new CMediaPlayerHistogramWidget(this);
  QHBoxLayout *histogramLayout = new QHBoxLayout;
  histogramLayout->addWidget(m_labelHistogram);
  histogramLayout->addWidget(m_videoHistogram, 1);
  histogramLayout->addWidget(m_audioHistogram, 2);

  m_videoProbe = new QVideoProbe(this);
  connect(m_videoProbe, &QVideoProbe::videoFrameProbed, m_videoHistogram, &CMediaPlayerHistogramWidget::ProcessFrame);
  m_videoProbe->setSource(m_player);

  m_audioProbe = new QAudioProbe(this);
  connect(m_audioProbe, &QAudioProbe::audioBufferProbed, m_audioHistogram, &CMediaPlayerHistogramWidget::ProcessBuffer);
  m_audioProbe->setSource(m_player);

  m_controlls = new CMediaPlayerControls(this);
  m_controlls->SetState(m_player->state());
  m_controlls->SetVolume(m_player->volume());
  m_controlls->SetMuted(m_controlls->IsMuted());

  connect(m_controlls, &CMediaPlayerControls::SignalPlay, m_player, &QMediaPlayer::play);
  connect(m_controlls, &CMediaPlayerControls::SignalPause, m_player, &QMediaPlayer::pause);
  connect(m_controlls, &CMediaPlayerControls::SignalStop, m_player, &QMediaPlayer::stop);
  connect(m_controlls, &CMediaPlayerControls::SignalChangeVolume, m_player, &QMediaPlayer::setVolume);
  connect(m_controlls, &CMediaPlayerControls::SignalChangeMuting, m_player, &QMediaPlayer::setMuted);
  connect(m_controlls, &CMediaPlayerControls::SignalChangeRate, m_player, &QMediaPlayer::setPlaybackRate);
  connect(m_controlls, &CMediaPlayerControls::SignalStop, m_videoWidget, QOverload<>::of(&QVideoWidget::update));

  connect(this, &CMediaPlayer::SignalPlay, m_player, &QMediaPlayer::play);
  connect(this, &CMediaPlayer::SignalPause, m_player, &QMediaPlayer::pause);
  connect(this, &CMediaPlayer::SignalStop, m_player, &QMediaPlayer::stop);
  connect(this, &CMediaPlayer::SignalChangeVolume, m_player, &QMediaPlayer::setVolume);
  connect(this, &CMediaPlayer::SignalChangeMuting, m_player, &QMediaPlayer::setMuted);
  connect(this, &CMediaPlayer::SignalChangeRate, m_player, &QMediaPlayer::setPlaybackRate);
  connect(this, &CMediaPlayer::SignalStop, m_videoWidget, QOverload<>::of(&QVideoWidget::update));

  connect(m_player, &QMediaPlayer::stateChanged, m_controlls, &CMediaPlayerControls::SetState);
  connect(m_player, &QMediaPlayer::volumeChanged, m_controlls, &CMediaPlayerControls::SetVolume);
  connect(m_player, &QMediaPlayer::mutedChanged, m_controlls, &CMediaPlayerControls::SetMuted);

  m_colorButton = new QPushButton(tr("Color Options..."), this);
  m_colorButton->setEnabled(false);
  connect(m_colorButton, &QPushButton::clicked, this, &CMediaPlayer::SlotShowColorDialog);

  QBoxLayout *displayLayout = new QHBoxLayout;
  displayLayout->addWidget(m_videoWidget, 2);

  QBoxLayout *controlLayout = new QHBoxLayout;
  controlLayout->setMargin(0);
  controlLayout->addStretch(1);
  controlLayout->addWidget(m_controlls);
  controlLayout->addStretch(1);
  controlLayout->addWidget(m_colorButton);

  QBoxLayout *layout = new QVBoxLayout;
  layout->addLayout(displayLayout);
  QHBoxLayout *hLayout = new QHBoxLayout;
  hLayout->addWidget(m_slider);
  hLayout->addWidget(m_labelDuration);
  layout->addLayout(hLayout);
  layout->addLayout(controlLayout);
  layout->addLayout(histogramLayout);

  setLayout(layout);

  if (!IsPlayerAvailable()) {
    QMessageBox::warning(this, tr("Service not available"),
                         tr("The QMediaPlayer object does not have a valid service.\n"\
                            "Please check the media service plugins are installed."));

    m_controlls->setEnabled(false);
    m_colorButton->setEnabled(false);
  }

  SlotMetaDataChanged();
}

CMediaPlayer::~CMediaPlayer()
{
}

//----------------------------------------------------------------------------------------
//
bool CMediaPlayer::IsPlayerAvailable() const
{
  return m_player->isAvailable();
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::SetPlaylist(const QUrl& url)
{
  m_playlist->clear();
  m_playlist->addMedia(url);
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::ShowControlls(bool bShow)
{
  m_controlls->setVisible(bShow);
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::SlotDrationChanged(qint64 duration)
{
  m_duration = duration / 1000;
  m_slider->setMaximum(static_cast<qint32>(m_duration));
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::SlotPositionChanged(qint64 progress)
{
  if (!m_slider->isSliderDown())
    m_slider->setValue(static_cast<qint32>(progress / 1000));

  UpdateDurationInfo(progress / 1000);
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::SlotMetaDataChanged()
{
  if (m_player->isMetaDataAvailable()) {
    SetTrackInfo(QString("%1 - %2")
                 .arg(m_player->metaData(QMediaMetaData::AlbumArtist).toString())
                 .arg(m_player->metaData(QMediaMetaData::Title).toString()));

    if (m_coverLabel) {
      QUrl url = m_player->metaData(QMediaMetaData::CoverArtUrlLarge).value<QUrl>();

      m_coverLabel->setPixmap(!url.isEmpty()
                              ? QPixmap(url.toString())
                              : QPixmap());
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::SlotPlaylistPositionChanged(int currentItem)
{
  Q_UNUSED(currentItem)
  ClearHistogram();
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::SlotSeek(int seconds)
{
  m_player->setPosition(seconds * 1000);
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::SlotStatusChanged(QMediaPlayer::MediaStatus status)
{
  HandleCursor(status);
  emit SignalStatusChanged(status);

  // handle status message
  switch (status) {
    case QMediaPlayer::UnknownMediaStatus:
    case QMediaPlayer::NoMedia:
    case QMediaPlayer::LoadedMedia:
      SetStatusInfo(QString());
      break;
    case QMediaPlayer::LoadingMedia:
      SetStatusInfo(tr("Loading..."));
      break;
    case QMediaPlayer::BufferingMedia:
    case QMediaPlayer::BufferedMedia:
      SetStatusInfo(tr("Buffering %1%").arg(m_player->bufferStatus()));
      break;
    case QMediaPlayer::StalledMedia:
      SetStatusInfo(tr("Stalled %1%").arg(m_player->bufferStatus()));
      break;
    case QMediaPlayer::EndOfMedia:
      //QApplication::alert(this);
      break;
    case QMediaPlayer::InvalidMedia:
      SlotDisplayErrorMessage();
      break;
  }
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::SlotStateChanged(QMediaPlayer::State state)
{
  if (state == QMediaPlayer::StoppedState)
    ClearHistogram();
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::HandleCursor(QMediaPlayer::MediaStatus status)
{
#ifndef QT_NO_CURSOR
  if (status == QMediaPlayer::LoadingMedia ||
      status == QMediaPlayer::BufferingMedia ||
      status == QMediaPlayer::StalledMedia)
    setCursor(QCursor(Qt::BusyCursor));
  else
    unsetCursor();
#endif
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::SlotBufferingProgress(int progress)
{
  if (m_player->mediaStatus() == QMediaPlayer::StalledMedia)
    SetStatusInfo(tr("Stalled %1%").arg(progress));
  else
    SetStatusInfo(tr("Buffering %1%").arg(progress));
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::SlotVideoAvailableChanged(bool available)
{
  m_videoWidget->setFullScreen(false);
  m_colorButton->setEnabled(available);
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::SetTrackInfo(const QString &info)
{
  m_trackInfo = info;

  if (m_statusBar) {
    m_statusBar->showMessage(m_trackInfo);
    m_statusLabel->setText(m_statusInfo);
  } else {
    if (!m_statusInfo.isEmpty())
      setWindowTitle(QString("%1 | %2").arg(m_trackInfo).arg(m_statusInfo));
    else
      setWindowTitle(m_trackInfo);
  }
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::SetStatusInfo(const QString &info)
{
  m_statusInfo = info;

  if (m_statusBar) {
    m_statusBar->showMessage(m_trackInfo);
    m_statusLabel->setText(m_statusInfo);
  } else {
    if (!m_statusInfo.isEmpty())
      setWindowTitle(QString("%1 | %2").arg(m_trackInfo).arg(m_statusInfo));
    else
      setWindowTitle(m_trackInfo);
  }
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::SlotDisplayErrorMessage()
{
  SetStatusInfo(m_player->errorString());
}

void CMediaPlayer::UpdateDurationInfo(qint64 currentInfo)
{
  QString tStr;
  if (currentInfo || m_duration) {
    QTime currentTime((currentInfo / 3600) % 60, (currentInfo / 60) % 60,
                      currentInfo % 60, (currentInfo * 1000) % 1000);
    QTime totalTime((m_duration / 3600) % 60, (m_duration / 60) % 60,
                    m_duration % 60, (m_duration * 1000) % 1000);
    QString format = "mm:ss";
    if (m_duration > 3600)
      format = "hh:mm:ss";
    tStr = currentTime.toString(format) + " / " + totalTime.toString(format);
  }
  m_labelDuration->setText(tStr);
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::SlotShowColorDialog()
{
  if (!m_colorDialog) {
    QSlider *brightnessSlider = new QSlider(Qt::Horizontal);
    brightnessSlider->setRange(-100, 100);
    brightnessSlider->setValue(m_videoWidget->brightness());
    connect(brightnessSlider, &QSlider::sliderMoved, m_videoWidget, &QVideoWidget::setBrightness);
    connect(m_videoWidget, &QVideoWidget::brightnessChanged, brightnessSlider, &QSlider::setValue);

    QSlider *contrastSlider = new QSlider(Qt::Horizontal);
    contrastSlider->setRange(-100, 100);
    contrastSlider->setValue(m_videoWidget->contrast());
    connect(contrastSlider, &QSlider::sliderMoved, m_videoWidget, &QVideoWidget::setContrast);
    connect(m_videoWidget, &QVideoWidget::contrastChanged, contrastSlider, &QSlider::setValue);

    QSlider *hueSlider = new QSlider(Qt::Horizontal);
    hueSlider->setRange(-100, 100);
    hueSlider->setValue(m_videoWidget->hue());
    connect(hueSlider, &QSlider::sliderMoved, m_videoWidget, &QVideoWidget::setHue);
    connect(m_videoWidget, &QVideoWidget::hueChanged, hueSlider, &QSlider::setValue);

    QSlider *saturationSlider = new QSlider(Qt::Horizontal);
    saturationSlider->setRange(-100, 100);
    saturationSlider->setValue(m_videoWidget->saturation());
    connect(saturationSlider, &QSlider::sliderMoved, m_videoWidget, &QVideoWidget::setSaturation);
    connect(m_videoWidget, &QVideoWidget::saturationChanged, saturationSlider, &QSlider::setValue);

    QFormLayout *layout = new QFormLayout;
    layout->addRow(tr("Brightness"), brightnessSlider);
    layout->addRow(tr("Contrast"), contrastSlider);
    layout->addRow(tr("Hue"), hueSlider);
    layout->addRow(tr("Saturation"), saturationSlider);

    QPushButton *button = new QPushButton(tr("Close"));
    layout->addRow(button);

    m_colorDialog = new QDialog(this);
    m_colorDialog->setWindowTitle(tr("Color Options"));
    m_colorDialog->setLayout(layout);

    connect(button, &QPushButton::clicked, m_colorDialog, &QDialog::close);
  }
  m_colorDialog->show();
}

//----------------------------------------------------------------------------------------
//
void CMediaPlayer::ClearHistogram()
{
  QMetaObject::invokeMethod(m_videoHistogram, "ProcessFrame", Qt::QueuedConnection, Q_ARG(QVideoFrame, QVideoFrame()));
  QMetaObject::invokeMethod(m_audioHistogram, "ProcessBuffer", Qt::QueuedConnection, Q_ARG(QAudioBuffer, QAudioBuffer()));
}
