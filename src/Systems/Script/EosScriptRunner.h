#ifndef CEOSSCRIPTRUNNER_H
#define CEOSSCRIPTRUNNER_H

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
class CJsonInstructionSetParser;
class CScriptRunnerSignalContext;
struct SJsonException;

//----------------------------------------------------------------------------------------
//
class CEosScriptRunnerInstanceController : public QObject,
                                           public IScriptRunnerInstanceController
{
  Q_OBJECT
  Q_INTERFACES(IScriptRunnerInstanceController)

public:
  CEosScriptRunnerInstanceController(const QString& sName,
                                     std::shared_ptr<CJsonInstructionSetRunner> spEosRunner);
  ~CEosScriptRunnerInstanceController() override;

  void InterruptExecution() override;
  bool IsRunning() const override;
  void RegisterNewComponent(const QString sName, CScriptRunnerSignalEmiter* pObject) override;
  void ResetEngine() override;
  void UnregisterComponents() override;

  std::shared_ptr<CJsonInstructionSetRunner> Runner() const;

signals:
  virtual void HandleScriptFinish(const QString& sName, bool bSuccess, const QVariant& sRetVal) override;

private:
  std::shared_ptr<CJsonInstructionSetRunner> m_spEosRunner;
};

//----------------------------------------------------------------------------------------
//
class CEosScriptRunner : public QObject, public IScriptRunnerFactory
{
  Q_OBJECT
  Q_INTERFACES(IScriptRunnerFactory)
  Q_DISABLE_COPY(CEosScriptRunner)

public:
  explicit CEosScriptRunner(std::weak_ptr<CScriptRunnerSignalContext> spSignalEmitterContext,
                            CScriptRunner* pRunnerParent,
                            QObject* pParent = nullptr);
  ~CEosScriptRunner() override;

  void Initialize() override;
  void Deinitialize() override;

  std::shared_ptr<IScriptRunnerInstanceController>
       LoadScript(const QString& sScript, tspScene spScene, tspResource spResource) override;
  void RegisterNewComponent(const QString sName, QJSValue signalEmitter) override;
  void UnregisterComponents() override;

  std::shared_ptr<IScriptRunnerInstanceController>
       OverlayRunAsync(const QString& sId, const QString& sScript,
                       tspResource spResource) override;

  void HandleScriptFinish(bool bSuccess, const QVariant& sRetVal);

signals:
  void SignalAddScriptRunner(const QString& sId, std::shared_ptr<IScriptRunnerInstanceController> spController) override;
  void SignalOverlayCleared() override;
  void SignalOverlayClosed(const QString& sId) override;
  void SignalOverlayRunAsync(tspProject spProject, const QString& sId,
                             const QString& sScriptResource) override;
  void SignalRemoveScriptRunner(const QString& sId) override;
  void SignalScriptRunFinished(bool bOk, const QString& sRetVal) override;

private slots:
  void SlotCommandRetVal(CJsonInstructionSetRunner::tRetVal retVal);
  void SlotFork(std::shared_ptr<CJsonInstructionSetRunner> spNewRunner, const QString& sForkCommandsName, bool bAutoRun);
  void SlotRun(std::shared_ptr<CJsonInstructionSetRunner> spEosRunnerMain,
               const QString& sRunner, const QString& sCommands);

private:
  void HandleError(const SJsonException& value);

  std::unique_ptr<CJsonInstructionSetParser>     m_spEosParser;
  std::weak_ptr<CScriptRunnerSignalContext>      m_wpSignalEmitterContext;
  mutable QMutex                                 m_sceneMutex;
  QString                                        m_sSceneName;
  mutable QMutex                                 m_objectMapMutex;
  std::map<QString /*name*/,
           std::shared_ptr<CScriptObjectBase>>   m_objectMap;
  CScriptRunner*                                 m_pRunnerParent;
  QAtomicInt                                     m_bInitialized;
};

#endif // CEOSSCRIPTRUNNER_H
