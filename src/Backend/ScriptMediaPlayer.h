#ifndef SCRIPTMEDIAPLAYER_H
#define SCRIPTMEDIAPLAYER_H

#include "Resource.h"
#include <QJSEngine>
#include <QJSValue>

class CScriptMediaPlayer : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptMediaPlayer)

public:
  CScriptMediaPlayer(QJSEngine* pEngine);
  ~CScriptMediaPlayer();

public slots:
  void show(QJSValue resource);
  void play(QJSValue resource);
  void playVideo(QJSValue resource = QJSValue());
  void pauseVideo();
  void stopVideo();
  void playSound(QJSValue resource = QJSValue());
  void pauseSound();
  void stopSound();

private:
  QJSEngine*               m_pEngine;
};

#endif // SCRIPTMEDIAPLAYER_H
