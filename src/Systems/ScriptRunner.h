#ifndef SCRIPTRUNNER_H
#define SCRIPTRUNNER_H

#include "ThreadedSystem.h"
#include "Script/IScriptRunner.h"
#include <QMutex>
#include <QPointer>
#include <QTimer>
#include <map>
#include <memory>

class CScriptRunnerSignalContext;
class CSettings;

//----------------------------------------------------------------------------------------
//
class CScriptRunner : public CSystemBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptRunner)

public:
  CScriptRunner();
  ~CScriptRunner() override;

  void InterruptExecution();
  void PauseExecution();
  void ResumeExecution();

  std::shared_ptr<CScriptRunnerSignalContext> SignalEmmitterContext();

signals:
  void SignalRunningChanged(bool bRunning);
  void SignalScriptRunFinished(bool bOk, const QString& sRetVal);

public slots:
  void Initialize() override;
  void Deinitialize() override;

  void LoadScript(tspScene spScene, tspResource spResource);
  void RegisterNewComponent(const QString sName, QJSValue signalEmitter);
  void UnregisterComponents();

private:
  std::map<QString, std::unique_ptr<IScriptRunner>> m_spRunnerMap;
  std::shared_ptr<CScriptRunnerSignalContext>       m_spSignalEmitterContext;
};

//----------------------------------------------------------------------------------------
//
class CScriptRunnerWrapper : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptRunnerWrapper)

public:
  CScriptRunnerWrapper(QObject* pParent, std::weak_ptr<CScriptRunner> wpRunner);
  ~CScriptRunnerWrapper() override;

  Q_INVOKABLE void pauseExecution();
  Q_INVOKABLE void registerNewComponent(const QString sName, QJSValue signalEmitter);
  Q_INVOKABLE void resumeExecution();

signals:
  void runningChanged(bool bRunning);

private:
  std::weak_ptr<CScriptRunner>                    m_wpRunner;
};

#endif // SCRIPTRUNNER_H
