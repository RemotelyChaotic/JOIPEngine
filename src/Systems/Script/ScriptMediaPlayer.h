#ifndef SCRIPTMEDIAPLAYER_H
#define SCRIPTMEDIAPLAYER_H

#include "ScriptObjectBase.h"
#include "ScriptRunnerSignalEmiter.h"
#include <QJSValue>
#include <memory>

class CDatabaseManager;
struct SResource;
typedef std::shared_ptr<SResource> tspResource;


class CMediaPlayerSignalEmitter : public CScriptRunnerSignalEmiter
{
  Q_OBJECT
public:
  CMediaPlayerSignalEmitter();
  ~CMediaPlayerSignalEmitter();

  virtual std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QPointer<QJSEngine> pEngine);

signals:
  void playVideo();
  void playSound();
  void pauseVideo();
  void pauseSound();
  void playMedia(const QString& sResource);
  void showMedia(const QString& sResource);
  void stopVideo();
  void stopSound();
  void startPlaybackWait();
  void startVideoWait();
  void startSoundWait();
  void playbackFinished();
  void videoFinished();
  void soundFinished();
};
Q_DECLARE_METATYPE(CMediaPlayerSignalEmitter)

//----------------------------------------------------------------------------------------
//
class CScriptMediaPlayer : public CScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptMediaPlayer)

public:
  CScriptMediaPlayer(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                     QPointer<QJSEngine> pEngine);
  ~CScriptMediaPlayer();

public slots:
  void show(QJSValue resource);
  void play();
  void play(QJSValue resource);
  void playVideo();
  void pauseVideo();
  void stopVideo();
  void playSound();
  void pauseSound();
  void stopSound();
  void waitForPlayback();
  void waitForVideo();
  void waitForSound();

private:
  std::weak_ptr<CDatabaseManager>  m_wpDbManager;
};

#endif // SCRIPTMEDIAPLAYER_H
