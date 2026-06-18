#ifndef CLUASCRIPTRUNNER_H
#define CLUASCRIPTRUNNER_H

#include "IScriptRunner.h"
#include "ScriptRunnerInstanceController.h"

#include <QObject>
#include <QPointer>
#include <QMutex>

#include <map>
#include <memory>
#include <set>
#include <vector>

class CProjectScriptWrapperReadOnly;
class CScriptRunnerInstanceController;
class CScriptRunnerSignalContext;
class CScriptRunnerSignalEmiter;
class CSceneScriptWrapperReadOnly;
class IScriptRunnerInstanceController;
namespace QtLua {
  class State;
  class String;
}
struct lua_State;

//----------------------------------------------------------------------------------------
//
class CLuaScriptRunner : public QObject, public IScriptRunnerFactory
{
  Q_OBJECT
  Q_INTERFACES(IScriptRunnerFactory)
  Q_DISABLE_COPY(CLuaScriptRunner)

public:
  using tRunningScriptsCheck = std::function<bool()>;

  CLuaScriptRunner(std::weak_ptr<CScriptRunnerSignalContext> spSignalEmitterContext,
                   tRunningScriptsCheck fnRunningScriptsCheck,
                   QObject* pParent = nullptr);
  ~CLuaScriptRunner() override;

  void Initialize() override;
  void Deinitialize() override;

  std::shared_ptr<IScriptRunnerInstanceController>
       LoadScript(const QString& sScript, tspScene spScene, tspResource spResource) override;
  void RegisterNewComponent(const QString& sName,
                            std::weak_ptr<CScriptCommunicator> wpCommunicator) override;
  void UnregisterComponent(const QString& sName) override;
  void UnregisterComponents() override;

  std::shared_ptr<IScriptRunnerInstanceController>
       RunAsync(const QString& sId, const QString& sScript,
                tspResource spResource) override;

signals:
  void SignalAddScriptRunner(const QString& sId,
                             std::shared_ptr<IScriptRunnerInstanceController> spController,
                             EScriptRunnerType type) override;
  void SignalClearThreads(EScriptRunnerType type) override;
  void SignalKill(const QString& sId) override;
  void SignalRunAsync(tspProject spProject, const QString& sId,
                      const QString& sScriptResource, EScriptRunnerType type) override;
  void SignalRemoveScriptRunner(const QString& sId) override;
  void SignalSceneLoaded(const QString& sScene) override;
  void SignalScriptRunFinished(bool bOk, const QString& sRetVal) override;

private slots:
  void SlotHandleScriptFinish(const QString& sName, bool bSuccess, const QVariant& sRetVal);

private:
  std::shared_ptr<CScriptRunnerInstanceController> CreateRunner(const QString& sId);
  void RunScript(std::shared_ptr<CScriptRunnerInstanceController> spRunner,
                 const QString& sScript,
                 tspScene spScene, tspResource spResource);

  std::weak_ptr<CScriptRunnerSignalContext>             m_wpSignalEmitterContext;
  mutable QMutex                                        m_communicatorMutex;
  std::map<QString, std::weak_ptr<CScriptCommunicator>> m_wpCommunicators;
  tRunningScriptsCheck                                  m_fnRunningScriptsCheck;
};

//----------------------------------------------------------------------------------------
//
class CScriptRunnerUtilsLua : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptRunnerUtilsLua)

public:
  CScriptRunnerUtilsLua(QObject* pParent,
                        QtLua::State* pState,
                        std::shared_ptr<CScriptRunnerSignalContext> pSignalEmiterContext);
  ~CScriptRunnerUtilsLua() override;

  void SetCurrentProject(tspProject spProject);

  // static address used for this
  static char sKeyThis;

public slots:
  QVariant include(const QString resource);
  void finishedScript(const QVariant& retVal);

signals:
  void finishedScriptSignal(const QVariant& sRetVal);

private:
  std::shared_ptr<CScriptRunnerSignalContext>     m_spSignalEmiterContext;
  tspProject                                      m_spProject;
  QtLua::State*                                   m_pState;
};

//----------------------------------------------------------------------------------------
//
namespace lua
{
  int LUACustomSearcher(lua_State* pL);
}

//----------------------------------------------------------------------------------------
//
class CLuaScriptRunnerInstanceWorker : public CScriptRunnerInstanceWorkerBase
{
  Q_OBJECT
  friend int lua::LUACustomSearcher(lua_State* pL);

public:
  CLuaScriptRunnerInstanceWorker(const QString& sName, bool bReadOnlyWrappers,
                                 std::weak_ptr<CScriptRunnerSignalContext> wpSignalEmitterContext);
  ~CLuaScriptRunnerInstanceWorker();

public slots:
  void HandleError(QtLua::String& sError);
  void Init() override;
  void Deinit() override;
  void InterruptExecution() override;
  void RunScript(const QString& sScript,
                 tspScene spScene, tspResource spResource) override;
  void RegisterNewComponent(const QString& sName,
                            std::weak_ptr<CScriptCommunicator> wpCommunicator) override;
  void ResetEngine() override;
  void UnregisterComponent(const QString& sName) override;
  bool IsInterrupted() const;
  tspProject CurrentProject() const;

private:
  QString GenerateEnvVariableString();
  void RegisterAbortHook(lua_State* pState);
  bool RegisterCustomPackageSearcher(lua_State* pState);
  bool OpenFileLibrary(QtLua::State* pState, const QString& sGlobal, const QString& sFile);
  void SetInterrupted(bool bInterrupted);

  tspProject                                     m_spProject;
  QPointer<QtLua::State>                         m_pLuaState;
  QPointer<CScriptRunnerUtilsLua>                m_pScriptUtils;
  QPointer<CSceneScriptWrapperReadOnly>          m_pCurrentScene;
  QPointer<CProjectScriptWrapperReadOnly>        m_pCurrentProject;
  std::map<QString /*name*/,
           std::shared_ptr<CScriptObjectBase>>   m_objectMap;
  std::vector<QString>                           m_vsObjectToDeleteMap;
  std::set<QString>                              m_vGlobalValues;
  QString                                        m_sName;
  QAtomicInt                                     m_bInterrupted;
  bool                                           m_bHandlingEvents = false;

protected:
  bool                                           m_bLoadingInbuiltLibraries;
};

#endif // CLUASCRIPTRUNNER_H
