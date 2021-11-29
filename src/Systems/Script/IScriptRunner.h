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

class IScriptRunner
{
public:
  IScriptRunner() = default;
  virtual ~IScriptRunner() = default;

  virtual void Initialize() = 0;
  virtual void Deinitialize() = 0;

  virtual void InterruptExecution() = 0;
  virtual void PauseExecution() = 0;
  virtual void ResumeExecution() = 0;

  virtual void LoadScript(const QString& sScript, tspScene spScene, tspResource spResource) = 0;
  virtual void RegisterNewComponent(const QString sName, QJSValue signalEmitter) = 0;
  virtual void UnregisterComponents() = 0;

signals:
  virtual void SignalScriptRunFinished(bool bOk, const QString& sRetVal) = 0;
};

Q_DECLARE_INTERFACE(IScriptRunner, "IScriptRunner")

#endif // ISCRIPTRUNNER_H
