#ifndef CJSSCRIPTRUNNER_H
#define CJSSCRIPTRUNNER_H

#include "IScriptRunner.h"
#include <QObject>
#include <QPointer>
#include <QJSEngine>
#include <QMutex>
#include <map>
#include <memory>

class CJsScriptRunnerInstanceController;
class CScriptRunnerUtils;
class CSceneScriptWrapper;
class CScriptRunnerSignalContext;
class CScriptRunnerSignalEmiter;

class CJsScriptRunner : public QObject, public IScriptRunner
{
  Q_OBJECT
  Q_INTERFACES(IScriptRunner)
  Q_DISABLE_COPY(CJsScriptRunner)
  friend class CScriptRunnerUtils;

public:
  explicit CJsScriptRunner(std::weak_ptr<CScriptRunnerSignalContext> spSignalEmitterContext,
                           QObject* pParent = nullptr);
  ~CJsScriptRunner() override;

  void Initialize() override;
  void Deinitialize() override;

  void InterruptExecution() override;
  void PauseExecution() override;
  void ResumeExecution() override;

  bool HasRunningScripts() const override;

  void LoadScript(const QString& sScript, tspScene spScene, tspResource spResource) override;
  void RegisterNewComponent(const QString sName, QJSValue signalEmitter) override;
  void UnregisterComponents() override;

signals:
  void SignalScriptRunFinished(bool bOk, const QString& sRetVal) override;

protected:
  std::shared_ptr<CScriptRunnerSignalContext> SignalEmmitterContext();

private slots:
  void SlotHandleScriptFinish(const QString& sName, bool bSuccess, const QVariant& sRetVal);
  void SlotOverlayCleared();
  void SlotOverlayClosed(const QString& sId);
  void SlotOverlayRunAsync(tspProject spProject, const QString& sId, const QString& sScriptResource);

private:
  void CreateRunner(const QString& sId);
  void RunScript(const QString& sId, const QString& sScript,
                 tspScene spScene, tspResource spResource);

  std::weak_ptr<CScriptRunnerSignalContext>      m_wpSignalEmitterContext;
  mutable QMutex                                 m_runnerMutex;
  std::map<QString, std::shared_ptr<CJsScriptRunnerInstanceController>>
                                                 m_vspJsRunner;
  mutable QMutex                                 m_signalEmiterMutex;
  std::map<QString, CScriptRunnerSignalEmiter*>  m_pSignalEmiters;
};


//----------------------------------------------------------------------------------------
//
class CScriptRunnerUtils : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptRunnerUtils)

public:
  CScriptRunnerUtils(QObject* pParent,
                     std::shared_ptr<CScriptRunnerSignalContext> pSignalEmiterContext);
  ~CScriptRunnerUtils() override;

  void SetCurrentProject(tspProject spProject);

public slots:
  QString include(QJSValue resource);

signals:
  void finishedScript(const QVariant& sRetVal);

private:
  std::shared_ptr<CScriptRunnerSignalContext>     m_spSignalEmiterContext;
  tspProject                                      m_spProject;
};

#endif // CJSSCRIPTRUNNER_H
