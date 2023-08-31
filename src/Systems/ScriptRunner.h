#ifndef SCRIPTRUNNER_H
#define SCRIPTRUNNER_H

#include "ThreadedSystem.h"
#include "Script/IScriptRunner.h"
#include <QMutex>
#include <QPointer>
#include <QTimer>

#include <functional>
#include <map>
#include <memory>

class CScriptRunnerSignalContext;
class CSettings;
class IScriptRunnerInstanceController;

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

  bool HasRunningScripts() const;
  std::shared_ptr<IScriptRunnerInstanceController> RunnerController(const QString& sId) const;
  std::shared_ptr<CScriptRunnerSignalContext> SignalEmmitterContext()const ;

signals:
  void SignalRunningChanged(bool bRunning);
  void SignalScriptRunFinished(bool bOk, const QString& sRetVal);

public slots:
  void Initialize() override;
  void Deinitialize() override;

  void LoadScript(tspScene spScene, tspResource spResource);
  void RegisterNewComponent(const QString sName, QJSValue signalEmitter);
  void UnregisterComponents();

protected slots:
  void SlotAddScriptController(const QString& sId, std::shared_ptr<IScriptRunnerInstanceController>);
  void SlotOverlayCleared();
  void SlotOverlayClosed(const QString& sId);
  void SlotOverlayRunAsync(tspProject spProject, const QString& sId, const QString& sScriptResource);
  void SlotRemoveScriptRunner(const QString& sId);
  void SlotScriptRunFinished(bool bOk, const QString& sRetVal);

private:
  void LoadScriptAndCall(tspScene spScene, tspResource spResource,
                         std::function<void(std::unique_ptr<IScriptRunnerFactory>&,
                                            const QString&, tspScene, tspResource)> fn);

  std::map<QString, std::unique_ptr<IScriptRunnerFactory>> m_spRunnerFactoryMap;
  mutable QMutex                                           m_runnerMutex;
  std::map<QString, std::shared_ptr<IScriptRunnerInstanceController>>
                                                           m_vspRunner;
  std::shared_ptr<CScriptRunnerSignalContext>              m_spSignalEmitterContext;
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
