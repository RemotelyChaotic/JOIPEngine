#ifndef CSCRIPTRUNNERINSTANCECONTROLLER_H
#define CSCRIPTRUNNERINSTANCECONTROLLER_H

#include "IScriptRunner.h"

#include "Systems/Database/Project.h"
#include "Systems/Database/Resource.h"
#include "Systems/Database/Scene.h"

#include <QObject>
#include <QVariant>
#include <map>
#include <memory>

class CScriptObjectBase;
class CScriptRunnerSignalContext;
class CScriptRunnerSignalEmiter;

class CScriptRunnerInstanceWorkerBase : public QObject
{
  Q_OBJECT
public:
  CScriptRunnerInstanceWorkerBase(const QString& sName,
                                  std::weak_ptr<CScriptRunnerSignalContext> wpSignalEmitterContext);
  ~CScriptRunnerInstanceWorkerBase();

  QAtomicInt                                     m_bRunning = 0;

public slots:
  virtual void FinishedScript(const QVariant& sRetVal);
  virtual void Init() = 0;
  virtual void Deinit() = 0;
  virtual void InterruptExecution() = 0;
  virtual void RunScript(const QString& sScript,
                         tspScene spScene, tspResource spResource) = 0;
  virtual void RegisterNewComponent(const QString& sName,
                                    std::weak_ptr<CScriptCommunicator> wpCommunicator) = 0;
  virtual void ResetEngine() = 0;
  virtual void UnregisterComponent(const QString& sName) = 0;
  virtual void UnregisterComponents();

signals:
  void HandleScriptFinish(bool bSuccess, const QVariant& sRetVal);
  void SignalInterruptExecution();
  void SignalClearThreads(EScriptRunnerType type);
  void SignalKill(const QString& sId);
  void SignalRunAsync(tspProject spProject, const QString& sId, const QString& sScriptResource,
                      EScriptRunnerType type);
  void SignalSceneLoaded(const QString& sScene);

protected:
  tspProject                                     m_spProject;
  std::weak_ptr<CScriptRunnerSignalContext>      m_wpSignalEmiterContext;
  std::map<QString /*name*/,
           std::shared_ptr<CScriptObjectBase>>   m_objectMap;
  QString                                        m_sName;
};

//----------------------------------------------------------------------------------------
//
class IScriptRunnerInstanceController
{
public:
  IScriptRunnerInstanceController() = default;
  virtual ~IScriptRunnerInstanceController() = default;

  virtual void InterruptExecution() = 0;
  virtual bool IsRunning() const = 0;
  virtual void RegisterNewComponent(const QString& sName,
                                    std::weak_ptr<CScriptCommunicator> wpCommunicator) = 0;
  virtual void ResetEngine() = 0;
  virtual void UnregisterComponent(const QString& sName) = 0;
  virtual void UnregisterComponents() = 0;

  // putting the signals here leads to compile errors because of MOC for no
  // reason whatsoever
//signals:
  virtual void HandleScriptFinish(const QString& sName, bool bSuccess, const QVariant& sRetVal) = 0;
};

Q_DECLARE_INTERFACE(IScriptRunnerInstanceController, "IScriptRunnerInstanceController")

//----------------------------------------------------------------------------------------
//
class CScriptRunnerInstanceController : public QObject,
                                        public IScriptRunnerInstanceController
{
  Q_OBJECT
  Q_INTERFACES(IScriptRunnerInstanceController)

public:
  CScriptRunnerInstanceController(const QString& sName,
                                  std::shared_ptr<CScriptRunnerInstanceWorkerBase> spWorkerBase,
                                  std::weak_ptr<CScriptRunnerSignalContext> wpSignalEmitterContext);
  ~CScriptRunnerInstanceController() override;

  void InterruptExecution() override;
  bool IsRunning() const override;
  void RegisterNewComponent(const QString& sName,
                            std::weak_ptr<CScriptCommunicator> wpCommunicator) override;
  void RunScript(const QString& sScript, tspScene spScene, tspResource spResource);
  void ResetEngine() override;
  void UnregisterComponent(const QString& sName) override;
  void UnregisterComponents() override;

signals:
  void HandleScriptFinish(const QString& sName, bool bSuccess, const QVariant& sRetVal) override;
  void SignalClearThreads(EScriptRunnerType type);
  void SignalKill(const QString& sId);
  void SignalRunAsync(tspProject spProject, const QString& sId, const QString& sScriptResource,
                      EScriptRunnerType type);
  void SignalSceneLoaded(const QString& sScene);

private slots:
  void SlotHandleScriptFinish(bool bSuccess, const QVariant& sRetVal);

private:
  std::shared_ptr<CScriptRunnerInstanceWorkerBase> m_spWorker;
  QPointer<QThread>                                m_pThread;
};

#endif // CSCRIPTRUNNERINSTANCECONTROLLER_H
