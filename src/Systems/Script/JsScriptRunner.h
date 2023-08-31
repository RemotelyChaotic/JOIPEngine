#ifndef CJSSCRIPTRUNNER_H
#define CJSSCRIPTRUNNER_H

#include "IScriptRunner.h"
#include <QObject>
#include <QPointer>
#include <QJSEngine>
#include <QMutex>
#include <map>
#include <memory>

class CScriptRunnerInstanceController;
class CScriptRunnerUtils;
class CScriptRunnerSignalContext;
class CScriptRunnerSignalEmiter;

class CJsScriptRunner : public QObject, public IScriptRunnerFactory
{
  Q_OBJECT
  Q_INTERFACES(IScriptRunnerFactory)
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

protected:
  std::shared_ptr<CScriptRunnerSignalContext> SignalEmmitterContext();

private slots:
  void SlotHandleScriptFinish(const QString& sName, bool bSuccess, const QVariant& sRetVal);

private:
  void CreateRunner(const QString& sId);
  void RunScript(const QString& sId, const QString& sScript,
                 tspScene spScene, tspResource spResource);

  std::weak_ptr<CScriptRunnerSignalContext>      m_wpSignalEmitterContext;
  mutable QMutex                                 m_runnerMutex;
  std::map<QString, std::shared_ptr<CScriptRunnerInstanceController>>
                                                 m_vspJsRunner;
  mutable QMutex                                 m_signalEmiterMutex;
  std::map<QString, CScriptRunnerSignalEmiter*>  m_pSignalEmiters;
};


//----------------------------------------------------------------------------------------
//
class CScriptRunnerUtilsJs : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptRunnerUtilsJs)

public:
  CScriptRunnerUtilsJs(QObject* pParent,
                     QPointer<QJSEngine> pEngine,
                     std::shared_ptr<CScriptRunnerSignalContext> pSignalEmiterContext);
  ~CScriptRunnerUtilsJs() override;

  void SetCurrentProject(tspProject spProject);

public slots:
  QJSValue include(QJSValue resource);
  QJSValue import(QJSValue resource);

signals:
  void finishedScript(const QVariant& sRetVal);

private:
  tspResource GetResource(QJSValue resource);

  std::shared_ptr<CScriptRunnerSignalContext>     m_spSignalEmiterContext;
  tspProject                                      m_spProject;
  QPointer<QJSEngine>                             m_pEngine;
};

#endif // CJSSCRIPTRUNNER_H
