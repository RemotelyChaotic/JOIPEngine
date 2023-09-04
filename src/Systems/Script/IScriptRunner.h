#ifndef ISCRIPTRUNNER_H
#define ISCRIPTRUNNER_H

#include <QObject>
#include <QJSValue>
#include <QVariant>

#include <memory>

class CScriptObjectBase;
class IScriptRunnerInstanceController;
struct SProject;
struct SResource;
struct SScene;
typedef std::shared_ptr<SProject> tspProject;
typedef std::shared_ptr<SResource> tspResource;
typedef std::shared_ptr<SScene> tspScene;

Q_DECLARE_METATYPE(std::shared_ptr<IScriptRunnerInstanceController>)

class IScriptRunnerFactory
{
public:
  static constexpr char c_sMainRunner[] = "~main";

  IScriptRunnerFactory() = default;
  virtual ~IScriptRunnerFactory() = default;

  virtual void Initialize() = 0;
  virtual void Deinitialize() = 0;

  virtual std::shared_ptr<IScriptRunnerInstanceController>
               LoadScript(const QString& sScript, tspScene spScene, tspResource spResource) = 0;
  virtual void RegisterNewComponent(const QString sName, QJSValue signalEmitter) = 0;
  virtual void UnregisterComponents() = 0;

  virtual std::shared_ptr<IScriptRunnerInstanceController>
               OverlayRunAsync(const QString& sId,
                               const QString& sScript, tspResource spResource) = 0;

signals:
  virtual void SignalAddScriptRunner(const QString& sId, std::shared_ptr<IScriptRunnerInstanceController> spController) = 0;
  virtual void SignalOverlayCleared() = 0;
  virtual void SignalOverlayClosed(const QString& sId) = 0;
  virtual void SignalOverlayRunAsync(tspProject spProject, const QString& sId,
                                     const QString& sScriptResource) = 0;
  virtual void SignalRemoveScriptRunner(const QString& sId) = 0;
  virtual void SignalSceneLoaded(const QString& sScene) = 0;
  virtual void SignalScriptRunFinished(bool bOk, const QString& sRetVal) = 0;
};

Q_DECLARE_INTERFACE(IScriptRunnerFactory, "IScriptRunnerFactory")

#endif // ISCRIPTRUNNER_H
