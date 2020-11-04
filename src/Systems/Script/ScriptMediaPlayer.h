#ifndef SCRIPTMEDIAPLAYER_H
#define SCRIPTMEDIAPLAYER_H

#include "ScriptObjectBase.h"
#include "ScriptRunnerSignalEmiter.h"
#include <QJSValue>
#include <memory>

class CDatabaseManager;


class CMediaPlayerSignalEmitter : public CScriptRunnerSignalEmiter
{
  Q_OBJECT
public:
  CMediaPlayerSignalEmitter();
  ~CMediaPlayerSignalEmitter();

  virtual std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QPointer<QJSEngine> pEngine);

signals:
  void playVideo();
  void playSound(const QString& sResource);
  void pauseVideo();
  void pauseSound(const QString& sResource);
  void playMedia(const QString& sResource);
  void showMedia(const QString& sResource);
  void stopVideo();
  void stopSound(const QString& sResource);
  void startPlaybackWait(const QString& sResource);
  void startVideoWait();
  void startSoundWait(const QString& sResource);
  void playbackFinished(const QString& sResource);
  void videoFinished();
  void soundFinished(const QString& sResource);
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
  void playSound(QJSValue resource);
  void pauseSound(QJSValue resource);
  void stopSound(QJSValue resource);
  void waitForPlayback();
  void waitForPlayback(QJSValue resource);
  void waitForVideo();
  void waitForSound();
  void waitForSound(QJSValue resource);

private:
  QString GetResourceName(const QJSValue& resource, bool* pbError = nullptr);
  void WaitForPlayBackImpl(const QString& sResource);
  void WaitForSoundImpl(const QString& sResource);

  std::weak_ptr<CDatabaseManager>  m_wpDbManager;
};

#endif // SCRIPTMEDIAPLAYER_H
