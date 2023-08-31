#ifndef CLUASCRIPTRUNNER_H
#define CLUASCRIPTRUNNER_H

#include "IScriptRunner.h"
#include <QObject>
#include <QPointer>
#include <QMutex>
#include <map>
#include <memory>

class CScriptRunnerInstanceController;
class CScriptRunnerSignalContext;
class CScriptRunnerSignalEmiter;
class IScriptRunnerInstanceController;
namespace QtLua {
  class State;
}

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
  void SignalScriptRunFinished(bool bOk, const QString& sRetVal) override;

private slots:
  void SlotHandleScriptFinish(const QString& sName, bool bSuccess, const QVariant& sRetVal);

private:
  std::shared_ptr<CScriptRunnerInstanceController> CreateRunner(const QString& sId);
  void RunScript(std::shared_ptr<CScriptRunnerInstanceController> spRunner,
                 const QString& sScript,
                 tspScene spScene, tspResource spResource);

  std::weak_ptr<CScriptRunnerSignalContext>      m_wpSignalEmitterContext;
  mutable QMutex                                 m_signalEmiterMutex;
  std::map<QString, CScriptRunnerSignalEmiter*>  m_pSignalEmiters;
  tRunningScriptsCheck                           m_fnRunningScriptsCheck;
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
  void finishedScript(const QVariant& retVal);

signals:
  void finishedScriptSignal(const QVariant& sRetVal);

private:
  std::shared_ptr<CScriptRunnerSignalContext>     m_spSignalEmiterContext;
  tspProject                                      m_spProject;
  QtLua::State*                                   m_pState;
};

#endif // CLUASCRIPTRUNNER_H
