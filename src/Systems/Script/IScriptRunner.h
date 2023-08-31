#ifndef ISCRIPTRUNNER_H
#define ISCRIPTRUNNER_H

#include <QObject>
#include <QJSValue>
#include <QVariant>

class CScriptObjectBase;
struct SProject;
struct SResource;
struct SScene;
typedef std::shared_ptr<SProject> tspProject;
typedef std::shared_ptr<SResource> tspResource;
typedef std::shared_ptr<SScene> tspScene;

class IScriptRunnerFactory
{
public:
  static constexpr char c_sMainRunner[] = "~main";

  IScriptRunnerFactory() = default;
  virtual ~IScriptRunnerFactory() = default;

  virtual void Initialize() = 0;
  virtual void Deinitialize() = 0;

  virtual void InterruptExecution() = 0;
  virtual void PauseExecution() = 0;
  virtual void ResumeExecution() = 0;

  virtual bool HasRunningScripts() const = 0;

  virtual void LoadScript(const QString& sScript, tspScene spScene, tspResource spResource) = 0;
  virtual void RegisterNewComponent(const QString sName, QJSValue signalEmitter) = 0;
  virtual void UnregisterComponents() = 0;

  virtual void OverlayCleared() = 0;
  virtual void OverlayClosed(const QString& sId)  = 0;
  virtual void OverlayRunAsync(const QString& sId,
                               const QString& sScript, tspResource spResource) = 0;

signals:
  virtual void SignalOverlayCleared() = 0;
  virtual void SignalOverlayClosed(const QString& sId) = 0;
  virtual void SignalOverlayRunAsync(tspProject spProject, const QString& sId,
                                     const QString& sScriptResource) = 0;
  virtual void SignalScriptRunFinished(bool bOk, const QString& sRetVal) = 0;
};

Q_DECLARE_INTERFACE(IScriptRunnerFactory, "IScriptRunnerFactory")

#endif // ISCRIPTRUNNER_H
