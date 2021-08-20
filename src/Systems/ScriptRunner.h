#ifndef SCRIPTRUNNER_H
#define SCRIPTRUNNER_H

#include "ThreadedSystem.h"
#include <QMutex>
#include <QPointer>
#include <QJSEngine>
#include <QTimer>
#include <memory>

class CScriptObjectBase;
class CScriptRunnerSignalContext;
class CScriptRunnerUtils;
class CSceneScriptWrapper;
class CSettings;
struct SProject;
struct SResource;
struct SScene;
typedef std::shared_ptr<SProject> tspProject;
typedef std::shared_ptr<SResource> tspResource;
typedef std::shared_ptr<SScene> tspScene;

//----------------------------------------------------------------------------------------
//
class CScriptRunner : public CSystemBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptRunner)
  friend class CScriptRunnerUtils;

public:
  CScriptRunner();
  ~CScriptRunner() override;

  void InterruptExecution();
  void PauseExecution();
  void ResumeExecution();

  std::shared_ptr<CScriptRunnerSignalContext> SignalEmmitterContext();

signals:
  void SignalRunningChanged(bool bRunning);
  void SignalScriptRunFinished(bool bOk, const QString& sRetVal);

public slots:
  void Initialize() override;
  void Deinitialize() override;

  void LoadScript(tspScene spScene, tspResource spResource);
  void RegisterNewComponent(const QString sName, QJSValue signalEmitter);
  void UnregisterComponents();

private slots:
  void SlotFinishedScript(const QVariant& sRetVal);
  void SlotRun();
  void SlotRegisterObject(const QString& sObject);

private:
  void HandleError(QJSValue& value);
  void HandleScriptFinish(bool bSuccess, const QVariant& sRetVal);

  std::shared_ptr<CSettings>                     m_spSettings;
  std::unique_ptr<QJSEngine>                     m_spScriptEngine;
  std::shared_ptr<CScriptRunnerSignalContext>    m_spSignalEmitterContext;
  std::shared_ptr<QTimer>                        m_spTimer;
  mutable QMutex                                 m_objectMapMutex;
  std::map<QString /*name*/,
           std::shared_ptr<CScriptObjectBase>>   m_objectMap;
  QPointer<CScriptRunnerUtils>                   m_pScriptUtils;
  QPointer<CSceneScriptWrapper>                               m_pCurrentScene;
  QJSValue                                       m_runFunction;
};

//----------------------------------------------------------------------------------------
//
class CScriptRunnerUtils : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptRunnerUtils)

public:
  CScriptRunnerUtils(QObject* pParent, QPointer<CScriptRunner> pScriptRunner);
  ~CScriptRunnerUtils() override;

  void SetCurrentProject(tspProject spProject);

public slots:
  QString include(QJSValue resource);

signals:
  void finishedScript(const QVariant& sRetVal);

private:
  QPointer<CScriptRunner>                         m_pScriptRunner;
  tspProject                                      m_spProject;
};

//----------------------------------------------------------------------------------------
//
class CScriptRunnerWrapper : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptRunnerWrapper)

public:
  CScriptRunnerWrapper(QObject* pParent, std::weak_ptr<CScriptRunner> wpRunner);
  ~CScriptRunnerWrapper() override;

  Q_INVOKABLE void pauseExecution();
  Q_INVOKABLE void registerNewComponent(const QString sName, QJSValue signalEmitter);
  Q_INVOKABLE void resumeExecution();

signals:
  void runningChanged(bool bRunning);

private:
  std::weak_ptr<CScriptRunner>                    m_wpRunner;
};

#endif // SCRIPTRUNNER_H
