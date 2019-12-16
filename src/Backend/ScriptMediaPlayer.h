#ifndef SCRIPTMEDIAPLAYER_H
#define SCRIPTMEDIAPLAYER_H

#include "Resource.h"
#include <QJSEngine>
#include <QJSValue>

class CDatabaseManager;
class CScriptRunnerSignalEmiter;
struct SProject;
typedef std::shared_ptr<SProject> tspProject;

class CScriptMediaPlayer : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptMediaPlayer)

public:
  CScriptMediaPlayer(std::shared_ptr<CScriptRunnerSignalEmiter> spEmitter,
                     QJSEngine* pEngine);
  ~CScriptMediaPlayer();

  void SetCurrentProject(tspProject spProject);

public slots:
  void show(QJSValue resource);
  void play();
  void play(QJSValue resource);
  void pauseVideo();
  void stopVideo();
  void pauseSound();
  void stopSound();

private:
  bool CheckIfScriptCanRun();

  std::shared_ptr<CScriptRunnerSignalEmiter> m_spSignalEmitter;
  tspProject                       m_spProject;
  std::weak_ptr<CDatabaseManager>  m_wpDbManager;
  QJSEngine*                       m_pEngine;
};

#endif // SCRIPTMEDIAPLAYER_H
