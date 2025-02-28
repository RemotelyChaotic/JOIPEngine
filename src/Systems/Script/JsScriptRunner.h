#ifndef CJSSCRIPTRUNNER_H
#define CJSSCRIPTRUNNER_H

#include "IScriptRunner.h"
#include <QObject>
#include <QPointer>
#include <QQmlAbstractUrlInterceptor>
#include <QQmlEngine>
#include <QMutex>

#include <functional>
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
  using tRunningScriptsCheck = std::function<bool()>;

  explicit CJsScriptRunner(std::weak_ptr<CScriptRunnerSignalContext> spSignalEmitterContext,
                           tRunningScriptsCheck fnRunningScriptsCheck,
                           QObject* pParent = nullptr);
  ~CJsScriptRunner() override;

  void Initialize() override;
  void Deinitialize() override;

  std::shared_ptr<IScriptRunnerInstanceController>
       LoadScript(const QString& sScript, tspScene spScene, tspResource spResource) override;
  void RegisterNewComponent(const QString sName, QJSValue signalEmitter) override;
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
  mutable QMutex                                 m_signalEmiterMutex;
  std::map<QString, CScriptRunnerSignalEmiter*>  m_pSignalEmiters;
  tRunningScriptsCheck                           m_fnRunningScriptsCheck;
};


//----------------------------------------------------------------------------------------
//
class CResourceUrlInterceptor : public QQmlAbstractUrlInterceptor
{
public:
  CResourceUrlInterceptor();
  ~CResourceUrlInterceptor() override;

  void SetCurrentProject(tspProject spProject);

  QUrl intercept(const QUrl& url, QQmlAbstractUrlInterceptor::DataType type) override;

  tspProject                                      m_spProject;
};

//----------------------------------------------------------------------------------------
//
class CScriptRunnerUtilsJs : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptRunnerUtilsJs)

public:
  CScriptRunnerUtilsJs(QObject* pParent,
                       QPointer<QQmlEngine> pEngine,
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
  QPointer<QQmlEngine>                            m_pEngine;
};

#endif // CJSSCRIPTRUNNER_H
