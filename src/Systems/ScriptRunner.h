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
class CScene;
class CSettings;
struct SResource;
struct SScene;
typedef std::shared_ptr<SResource> tspResource;
typedef std::shared_ptr<SScene> tspScene;


class CScriptRunner : public CSystemBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptRunner)

public:
  CScriptRunner();
  ~CScriptRunner() override;

  std::shared_ptr<CScriptRunnerSignalContext> SignalEmmitterContext();

signals:
  void SignalScriptRunFinished(bool bOk, const QString& sRetVal);

public slots:
  void Initialize() override;
  void Deinitialize() override;

  void LoadScript(tspScene spScene, tspResource spResource);
  void RegisterNewComponent(const QString sName, QJSValue signalEmitter);
  void UnregisterComponents();

private slots:
  void SlotRun();
  void SlotRegisterObject(const QString& sObject);

private:
  std::shared_ptr<CSettings>                     m_spSettings;
  std::unique_ptr<QJSEngine>                     m_spScriptEngine;
  std::shared_ptr<CScriptRunnerSignalContext>    m_spSignalEmitterContext;
  std::shared_ptr<QTimer>                        m_spTimer;
  mutable QMutex                                 m_objectMapMutex;
  std::map<QString /*name*/,
           std::shared_ptr<CScriptObjectBase>>   m_objectMap;
  QPointer<CScene>                               m_pCurrentScene;
  QJSValue                                       m_runFunction;
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

  Q_INVOKABLE void registerNewComponent(const QString sName, QJSValue signalEmitter);

private:
  std::weak_ptr<CScriptRunner>                    m_wpRunner;
};

#endif // SCRIPTRUNNER_H
