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

  std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QPointer<QJSEngine> pEngine) override;
  std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser) override;

signals:
  void playVideo();
  void playSound(const QString& sResource, const QString& sId,
                 qint64 iLoops, qint64 iStartAt);
  void pauseVideo();
  void pauseSound(const QString& sResource);
  void playMedia(const QString& sResource,
                 qint64 iLoops, qint64 iStartAt);
  void seekAudio(const QString& sResource, qint64 iSeek);
  void seekMedia(const QString& sResource, qint64 iSeek);
  void seekVideo(qint64 iSeek);
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
class CScriptMediaPlayer : public CJsScriptObjectBase
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
  void play(QJSValue resource, qint64 iLoops);
  void play(QJSValue resource, qint64 iLoops, qint64 iStartAt);
  void seek(QJSValue resource, qint64 iSeek);
  void playVideo();
  void pauseVideo();
  void seekVideo(qint64 iSeek);
  void stopVideo();
  void playSound(QJSValue resource);
  void playSound(QJSValue resource, const QString& sId);
  void playSound(QJSValue resource, const QString& sId, qint64 iLoops);
  void playSound(QJSValue resource, const QString& sId, qint64 iLoops, qint64 iStartAt);
  void pauseSound(QJSValue resource);
  void seekSound(QJSValue resource, qint64 iSeek);
  void stopSound(QJSValue resource);
  void waitForPlayback();
  void waitForPlayback(QJSValue resource);
  void waitForVideo();
  void waitForSound();
  void waitForSound(QJSValue resource);

private:
  QString GetResourceName(const QJSValue& resource, bool bStringCanBeId = false, bool* pbError = nullptr);
  void WaitForPlayBackImpl(const QString& sResource);
  void WaitForSoundImpl(const QString& sResource);

  std::weak_ptr<CDatabaseManager>  m_wpDbManager;
};

//----------------------------------------------------------------------------------------
//
class CCommandEosAudio;
class CCommandEosImage;
class CEosScriptMediaPlayer : public CEosScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CEosScriptMediaPlayer)

public:
  CEosScriptMediaPlayer(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                        QPointer<CJsonInstructionSetParser> pParser);
  ~CEosScriptMediaPlayer();

  void show(const QString& sResourceLocator);
  void playSound(const QString& sResourceLocator, const QString& iId = QString(),
            qint64 iLoops = 1, qint64 iStartAt = 0);
  void seek(const QString& iId = QString(), qint64 iSeek = 1);

private:
  QString GetResourceName(const QString& sResourceLocator, bool* pbError = nullptr);

  std::weak_ptr<CDatabaseManager>   m_wpDbManager;
  std::shared_ptr<CCommandEosImage> m_spCommandImg;
  std::shared_ptr<CCommandEosAudio> m_spCommandAudio;
};

#endif // SCRIPTMEDIAPLAYER_H
