#ifndef SCRIPTTHREAD_H
#define SCRIPTTHREAD_H

#include "ScriptObjectBase.h"
#include "ScriptRunnerSignalEmiter.h"
#include <QJSValue>
#include <memory>

class CDatabaseManager;

class CThreadSignalEmitter : public CScriptRunnerSignalEmiter
{
  Q_OBJECT
public:
  CThreadSignalEmitter();
  ~CThreadSignalEmitter();

signals:
  void skippableWait(qint32 iTimeS);
  void waitSkipped();

protected:
  std::shared_ptr<CScriptCommunicator>
  CreateCommunicatorImpl(std::shared_ptr<CScriptRunnerSignalEmiterAccessor> spAccessor) override;
};

//----------------------------------------------------------------------------------------
//
class CThreadScriptCommunicator : public CScriptCommunicator
{
  public:
  CThreadScriptCommunicator(const std::weak_ptr<CScriptRunnerSignalEmiterAccessor>& spEmitter);
  ~CThreadScriptCommunicator() override;

  CScriptObjectBase* CreateNewScriptObject(QPointer<QJSEngine> pEngine) override;
  CScriptObjectBase* CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser) override;
  CScriptObjectBase* CreateNewScriptObject(QtLua::State* pState) override;
  CScriptObjectBase* CreateNewSequenceObject() override;
};

//----------------------------------------------------------------------------------------
//
class CScriptThread : public CJsScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptThread)

public:
  CScriptThread(std::weak_ptr<CScriptCommunicator> pCommunicator,
                QPointer<QJSEngine> pEngine);
  CScriptThread(std::weak_ptr<CScriptCommunicator> pCommunicator,
                QtLua::State* pState);
  ~CScriptThread();

public slots:
  void kill(QString sId);
  void runAsynch(QVariant resource);
  void runAsynch(QVariant resource, QVariant sId);
  void sleep(qint32 iTimeS);
  void sleep(qint32 iTimeS, QVariant bSkippable);

signals:
  void SignalOverlayRunAsync(const QString& sId, const QString& sScriptResource);
  void SignalKill(const QString& sId);
  void SignalQuitLoop();
  void SignalPauseTimer();
  void SignalResumeTimer();

private:
  QString GetResourceName(const QVariant& resource, const QString& sMethod,
                          bool bStringCanBeId = false, bool* pbOk = nullptr);

  std::shared_ptr<std::function<void()>> m_spStop;
  std::shared_ptr<std::function<void()>> m_spPause;
  std::shared_ptr<std::function<void()>> m_spResume;
  std::weak_ptr<CDatabaseManager>  m_wpDbManager;
};

#endif // SCRIPTTHREAD_H
