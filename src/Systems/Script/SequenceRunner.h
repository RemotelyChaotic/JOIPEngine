#ifndef CSEQUENCERUNNERINSTANCECONTROLLER_H
#define CSEQUENCERUNNERINSTANCECONTROLLER_H

#include "IScriptRunner.h"

#include "Systems/JSON/JsonInstructionSetRunner.h"

#include "Systems/Script/ScriptRunnerInstanceController.h"

#include <QObject>
#include <QMutex>
#include <QPointer>
#include <map>
#include <memory>
#include <map>

class CScriptRunner;

//----------------------------------------------------------------------------------------
//
class CSequenceRunner : public QObject, public IScriptRunnerFactory
{
  Q_OBJECT
  Q_INTERFACES(IScriptRunnerFactory)
  Q_DISABLE_COPY(CSequenceRunner)

public:
  using tRunningScriptsCheck = std::function<bool()>;

  explicit CSequenceRunner(std::weak_ptr<CScriptRunnerSignalContext> spSignalEmitterContext,
                           tRunningScriptsCheck fnRunningScriptsCheck,
                           QObject* pParent = nullptr);
  ~CSequenceRunner() override;

  void Initialize() override;
  void Deinitialize() override;

  std::shared_ptr<IScriptRunnerInstanceController>
  LoadScript(const QString& sScript, tspScene spScene, tspResource spResource) override;
  void RegisterNewComponent(const QString sName, QJSValue signalEmitter) override;
  void UnregisterComponents() override;

  std::shared_ptr<IScriptRunnerInstanceController>
  OverlayRunAsync(const QString& sId, const QString& sScript,
                  tspResource spResource) override;

signals:
  void SignalAddScriptRunner(const QString& sId, std::shared_ptr<IScriptRunnerInstanceController> spController) override;
  void SignalOverlayCleared() override;
  void SignalOverlayClosed(const QString& sId) override;
  void SignalOverlayRunAsync(tspProject spProject, const QString& sId,
                             const QString& sScriptResource) override;
  void SignalRemoveScriptRunner(const QString& sId) override;
  void SignalSceneLoaded(const QString& sScene) override;
  void SignalScriptRunFinished(bool bOk, const QString& sRetVal) override;

protected:
  std::shared_ptr<CScriptRunnerSignalContext> SignalEmmitterContext();

private slots:
  void SlotHandleScriptFinish(const QString& sName, bool bSuccess, const QVariant& sRetVal);

private:
  std::shared_ptr<CScriptRunnerInstanceController> CreateRunner(const QString& sId);
  void RunScript(std::shared_ptr<CScriptRunnerInstanceController> spController,
                 const QString& sScript,
                 tspScene spScene, tspResource spResource);

  std::weak_ptr<CScriptRunnerSignalContext>      m_wpSignalEmitterContext;
  CScriptRunner*                                 m_pRunnerParent;
  mutable QMutex                                 m_signalEmiterMutex;
  std::map<QString, CScriptRunnerSignalEmiter*>  m_pSignalEmiters;
  tRunningScriptsCheck                           m_fnRunningScriptsCheck;
  QAtomicInt                                     m_bInitialized;
};

#endif // CSEQUENCERUNNERINSTANCECONTROLLER_H
