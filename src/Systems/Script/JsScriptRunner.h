#ifndef CJSSCRIPTRUNNER_H
#define CJSSCRIPTRUNNER_H

#include "IScriptRunner.h"
#include <QObject>
#include <QPointer>
#include <QJSEngine>
#include <QMutex>

class CScriptRunnerUtils;
class CSceneScriptWrapper;
class CScriptRunnerSignalContext;

class CJsScriptRunner : public QObject, public IScriptRunner
{
  Q_OBJECT
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

  void LoadScript(const QString& sScript, tspScene spScene, tspResource spResource) override;
  void RegisterNewComponent(const QString sName, QJSValue signalEmitter) override;
  void UnregisterComponents() override;
  void HandleScriptFinish(bool bSuccess, const QVariant& sRetVal) override;

signals:
  void SignalScriptRunFinished(bool bOk, const QString& sRetVal) override;

protected:
  std::shared_ptr<CScriptRunnerSignalContext> SignalEmmitterContext();

private slots:
  void SlotFinishedScript(const QVariant& sRetVal);
  void SlotRegisterObject(const QString& sObject);
  void SlotRun();

private:
  void HandleError(QJSValue& value);

  std::shared_ptr<QTimer>                        m_spTimer;
  std::weak_ptr<CScriptRunnerSignalContext>      m_wpSignalEmitterContext;
  QPointer<QJSEngine>                            m_pScriptEngine;
  QPointer<CScriptRunnerUtils>                   m_pScriptUtils;
  QPointer<CSceneScriptWrapper>                  m_pCurrentScene;
  QJSValue                                       m_runFunction;
  mutable QMutex                                 m_objectMapMutex;
  std::map<QString /*name*/,
           std::shared_ptr<CScriptObjectBase>>   m_objectMap;
};


//----------------------------------------------------------------------------------------
//
class CScriptRunnerUtils : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptRunnerUtils)

public:
  CScriptRunnerUtils(QObject* pParent, QPointer<CJsScriptRunner> pScriptRunner);
  ~CScriptRunnerUtils() override;

  void SetCurrentProject(tspProject spProject);

public slots:
  QString include(QJSValue resource);

signals:
  void finishedScript(const QVariant& sRetVal);

private:
  QPointer<CJsScriptRunner>                       m_pScriptRunner;
  tspProject                                      m_spProject;
};

#endif // CJSSCRIPTRUNNER_H
