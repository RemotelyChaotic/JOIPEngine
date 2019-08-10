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

#ifndef PLAYERWINDOW_H
#define PLAYERWINDOW_H

#include <QWidget>
#include <QtAV>

QT_BEGIN_NAMESPACE
class QSlider;
class QPushButton;
QT_END_NAMESPACE
class CMediaPlayer : public QWidget
{
  Q_OBJECT
public:
  explicit CMediaPlayer(QWidget* pParent = nullptr);
  ~CMediaPlayer() override;

  void SetSliderVisible(bool bVisible);

public slots:
  void OpenMedia(const QString sPath);
  void SeekBySlider(qint32 value);
  void SeekBySlider();
  void PlayPause();
  void Stop();

private slots:
  void UpdateSlider(qint64 value);
  void UpdateSlider();
  void UpdateSliderUnit();

private:
  QtAV::VideoOutput* m_vo;
  QtAV::AVPlayer* m_player;
  QSlider* m_slider;
  int m_unit;
};

#endif // PLAYERWINDOW_H
