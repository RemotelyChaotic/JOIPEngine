#ifndef CEOSSCRIPTRUNNER_H
#define CEOSSCRIPTRUNNER_H

#include "IScriptRunner.h"
#include <QObject>
#include <QMutex>
#include <QPointer>
#include <QTimer>
#include <map>
#include <memory>

class CJsonInstructionSetParser;
class CJsonInstructionSetRunner;
class CScriptRunnerSignalContext;
struct SJsonException;

class CEosScriptRunner : public QObject, public IScriptRunner
{
  Q_OBJECT
  Q_INTERFACES(IScriptRunner)
  Q_DISABLE_COPY(CEosScriptRunner)

public:
  explicit CEosScriptRunner(std::weak_ptr<CScriptRunnerSignalContext> spSignalEmitterContext,
                            QObject* pParent = nullptr);
  ~CEosScriptRunner();

  void Initialize() override;
  void Deinitialize() override;

  void InterruptExecution() override;
  void PauseExecution() override;
  void ResumeExecution() override;

  void LoadScript(const QString& sScript, tspScene spScene, tspResource spResource) override;
  void RegisterNewComponent(const QString sName, QJSValue signalEmitter) override;
  void UnregisterComponents() override;
  void HandleScriptFinish(bool bSuccess, const QVariant& sRetVal) override;

signals:
  void SignalScriptRunFinished(bool bOk, const QString& sRetVal) override;

private slots:
  void SlotRegisterObject(const QString& sObject);
  void SlotRun();

private:
  void HandleError(const SJsonException& value);

  std::unique_ptr<CJsonInstructionSetParser>     m_spEosParser;
  std::shared_ptr<CJsonInstructionSetRunner>     m_spEosRunner;
  std::shared_ptr<QTimer>                        m_spTimer;
  std::weak_ptr<CScriptRunnerSignalContext>      m_wpSignalEmitterContext;
  tspScene                                       m_spCurrentScene;
  mutable QMutex                                 m_sceneMutex;
  QString                                        m_sSceneName;
  mutable QMutex                                 m_objectMapMutex;
  std::map<QString /*name*/,
           std::shared_ptr<CScriptObjectBase>>   m_objectMap;
};

#endif // CEOSSCRIPTRUNNER_H