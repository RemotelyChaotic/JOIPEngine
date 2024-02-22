#ifndef SCRIPTMEDIAPLAYER_H
#define SCRIPTMEDIAPLAYER_H

#include "ScriptObjectBase.h"
#include "ScriptRunnerSignalEmiter.h"
#include <QVariant>
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
  std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QtLua::State* pState) override;
  std::shared_ptr<CScriptObjectBase> CreateNewSequenceObject() override;

signals:
  void playVideo();
  void playSound(const QString& sResource, const QString& sId,
                 qint64 iLoops, qint64 iStartAt, qint64 iEndAt);
  void pauseVideo();
  void pauseSound(const QString& sResource);
  void playMedia(const QString& sResource,
                 qint64 iLoops, qint64 iStartAt, qint64 iEndAt);
  void seekAudio(const QString& sResource, qint64 iSeek);
  void seekMedia(const QString& sResource, qint64 iSeek);
  void seekVideo(qint64 iSeek);
  void setVolume(const QString& sResource, double dValue);
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
  CScriptMediaPlayer(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                     QtLua::State* pState);
  ~CScriptMediaPlayer();

public slots:
  void show(QVariant resource);
  void play();
  void play(QVariant resource);
  void play(QVariant resource, qint64 iLoops);
  void play(QVariant resource, qint64 iLoops, qint64 iStartAt);
  void play(QVariant resource, qint64 iLoops, qint64 iStartAt, qint64 iEndAt);
  void seek(QVariant resource, qint64 iSeek);
  void playVideo();
  void pauseVideo();
  void seekVideo(qint64 iSeek);
  void stopVideo();
  void playSound(QVariant resource);
  void playSound(QVariant resource, const QString& sId);
  void playSound(QVariant resource, const QString& sId, qint64 iLoops);
  void playSound(QVariant resource, const QString& sId, qint64 iLoops, qint64 iStartAt);
  void playSound(QVariant resource, const QString& sId, qint64 iLoops, qint64 iStartAt, qint64 iEndAt);
  void pauseSound(QVariant resource);
  void seekSound(QVariant resource, qint64 iSeek);
  void stopSound(QVariant resource);
  void setVolume(double dValue);
  void setVolume(QVariant resource, double dValue);
  void waitForPlayback();
  void waitForPlayback(QVariant resource);
  void waitForVideo();
  void waitForSound();
  void waitForSound(QVariant resource);

private:
  QString GetResourceName(const QVariant& resource, const QString& sMethod,
                          bool bStringCanBeId = false, bool* pbOk = nullptr);
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
  void setVolume(const QString& iId, double dValue);

private:
  QString GetResourceName(const QString& sResourceLocator, bool* pbError = nullptr);

  std::weak_ptr<CDatabaseManager>   m_wpDbManager;
  std::shared_ptr<CCommandEosImage> m_spCommandImg;
  std::shared_ptr<CCommandEosAudio> m_spCommandAudio;
};

#endif // SCRIPTMEDIAPLAYER_H
