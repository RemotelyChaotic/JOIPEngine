#ifndef SCRIPTRUNNER_H
#define SCRIPTRUNNER_H

#include "ThreadedSystem.h"
#include <QJSEngine>
#include <QTimer>
#include <memory>

class CScriptBackground;
class CScriptIcon;
class CScriptMediaPlayer;
class CScriptRunnerSignalEmiter;
class CScriptStorage;
class CScriptTextBox;
class CScriptTimer;
class CScriptThread;
class CSettings;
struct SResource;
struct SScene;
typedef std::shared_ptr<SResource> tspResource;
typedef std::shared_ptr<SScene> tspScene;

class CScriptRunner : public CThreadedObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptRunner)

public:
  CScriptRunner();
  ~CScriptRunner() override;

  std::shared_ptr<CScriptRunnerSignalEmiter> SignalEmmitter();

signals:
  void SignalScriptRunFinished(bool bOk, const QString& sRetVal);

public slots:
  void Initialize() override;
  void Deinitialize() override;
  void SlotLoadScript(tspScene spScene, tspResource spResource);
  void SlotRun();

private:
  std::shared_ptr<CSettings>             m_spSettings;
  std::unique_ptr<QJSEngine>             m_spScriptEngine;
  std::shared_ptr<CScriptRunnerSignalEmiter> m_spSignalEmitter;
  std::shared_ptr<QTimer>                m_spTimer;
  CScriptBackground*                     m_pBackgroundObject;
  CScriptIcon*                           m_pScriptIcon;
  CScriptMediaPlayer*                    m_pMediaPlayerObject;
  CScriptStorage*                        m_pStorageObject;
  CScriptTextBox*                        m_pTextBoxObject;
  CScriptTimer*                          m_pTimerObject;
  CScriptThread*                         m_pThreadObject;
  QJSValue                               m_runFunction;
};

#endif // SCRIPTRUNNER_H
