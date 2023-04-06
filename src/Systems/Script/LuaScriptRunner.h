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
namespace QtLua {
  class State;
}

class CLuaScriptRunner : public QObject, public IScriptRunner
{
  Q_OBJECT
  Q_INTERFACES(IScriptRunner)
  Q_DISABLE_COPY(CLuaScriptRunner)

public:
  CLuaScriptRunner(std::weak_ptr<CScriptRunnerSignalContext> spSignalEmitterContext,
                   QObject* pParent = nullptr);
  ~CLuaScriptRunner() override;

  void Initialize() override;
  void Deinitialize() override;

  void InterruptExecution() override;
  void PauseExecution() override;
  void ResumeExecution() override;

  bool HasRunningScripts() const override;

  void LoadScript(const QString& sScript, tspScene spScene, tspResource spResource) override;
  void RegisterNewComponent(const QString sName, QJSValue signalEmitter) override;
  void UnregisterComponents() override;

  void OverlayCleared() override;
  void OverlayClosed(const QString& sId) override;
  void OverlayRunAsync(const QString& sId, const QString& sScript,
                       tspResource spResource) override;

signals:
  void SignalOverlayCleared() override;
  void SignalOverlayClosed(const QString& sId) override;
  void SignalOverlayRunAsync(tspProject spProject, const QString& sId,
                             const QString& sScriptResource) override;
  void SignalScriptRunFinished(bool bOk, const QString& sRetVal) override;

private slots:
  void SlotHandleScriptFinish(const QString& sName, bool bSuccess, const QVariant& sRetVal);

private:
  void CreateRunner(const QString& sId);
  void RunScript(const QString& sId, const QString& sScript,
                 tspScene spScene, tspResource spResource);

  std::weak_ptr<CScriptRunnerSignalContext>      m_wpSignalEmitterContext;
  mutable QMutex                                 m_runnerMutex;
  std::map<QString, std::shared_ptr<CScriptRunnerInstanceController>>
                                                 m_vspLuaRunner;
  mutable QMutex                                 m_signalEmiterMutex;
  std::map<QString, CScriptRunnerSignalEmiter*>  m_pSignalEmiters;
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

signals:
  void finishedScript(const QVariant& sRetVal);

private:
  std::shared_ptr<CScriptRunnerSignalContext>     m_spSignalEmiterContext;
  tspProject                                      m_spProject;
  QtLua::State*                                   m_pState;
};

#endif // CLUASCRIPTRUNNER_H
