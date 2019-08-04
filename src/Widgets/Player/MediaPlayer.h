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

#ifndef PLAYER_H
#define PLAYER_H

#include <QWidget>
#include <QMediaPlayer>
#include <QMediaPlaylist>

QT_BEGIN_NAMESPACE
class QAbstractItemView;
class QLabel;
class QMediaPlayer;
class QModelIndex;
class QPushButton;
class QSlider;
class QStatusBar;
class QVideoProbe;
class QVideoWidget;
class QAudioProbe;
QT_END_NAMESPACE

class CMediaPlayerControls;
class CMediaPlayerHistogramWidget;

class CMediaPlayer : public QWidget
{
  Q_OBJECT

public:
  explicit CMediaPlayer(QWidget *parent = nullptr);
  ~CMediaPlayer();

  bool IsPlayerAvailable() const;
  void SetPlaylist(const QUrl& url);
  void ShowControlls(bool bShow);

signals:
  void SignalPlay();
  void SignalPause();
  void SignalStop();
  void SignalChangeVolume(int Volume);
  void SignalChangeMuting(bool muting);
  void SignalChangeRate(qreal rate);
  void SignalStatusChanged(QMediaPlayer::MediaStatus status);

private slots:
  void SlotDrationChanged(qint64 duration);
  void SlotPositionChanged(qint64 progress);
  void SlotMetaDataChanged();

  void SlotSeek(int seconds);
  void SlotPlaylistPositionChanged(int);

  void SlotStatusChanged(QMediaPlayer::MediaStatus status);
  void SlotStateChanged(QMediaPlayer::State state);
  void SlotBufferingProgress(int progress);
  void SlotVideoAvailableChanged(bool available);

  void SlotDisplayErrorMessage();

  void SlotShowColorDialog();

private:
  void ClearHistogram();
  void SetTrackInfo(const QString &info);
  void SetStatusInfo(const QString &info);
  void HandleCursor(QMediaPlayer::MediaStatus status);
  void UpdateDurationInfo(qint64 currentInfo);

  QMediaPlayer* m_player = nullptr;
  CMediaPlayerControls* m_controlls = nullptr;
  QMediaPlaylist* m_playlist = nullptr;
  QVideoWidget* m_videoWidget = nullptr;
  QLabel* m_coverLabel = nullptr;
  QSlider* m_slider = nullptr;
  QLabel* m_labelDuration = nullptr;
  QPushButton* m_colorButton = nullptr;
  QDialog* m_colorDialog = nullptr;
  QLabel* m_statusLabel = nullptr;
  QStatusBar* m_statusBar = nullptr;

  QLabel* m_labelHistogram = nullptr;
  CMediaPlayerHistogramWidget* m_videoHistogram = nullptr;
  CMediaPlayerHistogramWidget* m_audioHistogram = nullptr;
  QVideoProbe* m_videoProbe = nullptr;
  QAudioProbe* m_audioProbe = nullptr;

  QString m_trackInfo;
  QString m_statusInfo;
  qint64 m_duration;
};

#endif // PLAYER_H
