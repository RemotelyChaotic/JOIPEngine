#ifndef SCRIPTMEDIAPLAYER_H
#define SCRIPTMEDIAPLAYER_H

#include "ScriptObjectBase.h"
#include <QJSValue>
#include <memory>

class CDatabaseManager;

class CScriptMediaPlayer : public CScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptMediaPlayer)

public:
  CScriptMediaPlayer(std::shared_ptr<CScriptRunnerSignalEmiter> spEmitter,
                     QPointer<QJSEngine> pEngine);
  ~CScriptMediaPlayer();

public slots:
  void show(QJSValue resource);
  void play();
  void play(QJSValue resource);
  void pauseVideo();
  void stopVideo();
  void pauseSound();
  void stopSound();
  void waitForPlayback();

private:
  std::weak_ptr<CDatabaseManager>  m_wpDbManager;
};

#endif // SCRIPTMEDIAPLAYER_H
