#ifndef SCRIPTTIMER_H
#define SCRIPTTIMER_H

#include "CommonScriptHelpers.h"
#include "ScriptObjectBase.h"
#include "ScriptRunnerSignalEmiter.h"
#include <QJSValue>
#include <memory>


class CTimerSignalEmitter : public CScriptRunnerSignalEmiter
{
  Q_OBJECT
public:
  CTimerSignalEmitter();
  ~CTimerSignalEmitter();

signals:
  void hideTimer();
  void setTime(double dTimeS);
  void setTimeVisible(bool bVisible);
  void showTimer();
  void startTimer();
  void stopTimer();
  void waitForTimer();
  void timerFinished();

protected:
  std::shared_ptr<CScriptCommunicator>
  CreateCommunicatorImpl(std::shared_ptr<CScriptRunnerSignalEmiterAccessor> spAccessor) override;
};

//----------------------------------------------------------------------------------------
//
class CTimerScriptCommunicator : public CScriptCommunicator
{
  public:
  CTimerScriptCommunicator(const std::weak_ptr<CScriptRunnerSignalEmiterAccessor>& spEmitter);
  ~CTimerScriptCommunicator() override;

  CScriptObjectBase* CreateNewScriptObject(QPointer<QJSEngine> pEngine) override;
  CScriptObjectBase* CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser) override;
  CScriptObjectBase* CreateNewScriptObject(QtLua::State* pState) override;
  CScriptObjectBase* CreateNewSequenceObject() override;
};

//----------------------------------------------------------------------------------------
//
class CScriptTimer : public CJsScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptTimer)

public:
  CScriptTimer(std::weak_ptr<CScriptCommunicator> pCommunicator,
               QPointer<QJSEngine> pEngine);
  CScriptTimer(std::weak_ptr<CScriptCommunicator> pCommunicator,
               QtLua::State* pState);
  ~CScriptTimer();

public slots:
  void hide();
  void setTime(double dTimeS);
  void setTimeVisible(bool bVisible);
  void show();
  void start();
  void stop();
  void waitForTimer();

  void registerTimerFinishedCallback(QVariant callback);

signals:
  void SignalQuitLoop();

private slots:
  void HandleTimerFinished();

private:
  std::shared_ptr<std::function<void()>> m_spStop;
  script::tCallbackValue m_callback;
};

//----------------------------------------------------------------------------------------
//
class CCommandEosTimer;
class CEosScriptTimer : public CEosScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CEosScriptTimer)

public:
  CEosScriptTimer(std::weak_ptr<CScriptCommunicator> pCommunicator,
                  QPointer<CJsonInstructionSetParser> pParser);
  ~CEosScriptTimer();

  QString getTimerValue(QString sValue);
  void hide();
  void setTime(double dTimeS);
  void setTimeVisible(bool bVisible);
  void sleep(qint64 iTimeMs);
  void show();
  void start();
  void waitForTimer();

signals:
  void SignalQuitLoop();
  void SignalPauseTimer();
  void SignalResumeTimer();

private:
  std::shared_ptr<CCommandEosTimer> m_spCommandTimer;
  std::shared_ptr<std::function<void()>> m_spStop;
  std::shared_ptr<std::function<void()>> m_spPause;
  std::shared_ptr<std::function<void()>> m_spResume;
};

#endif // SCRIPTTIMER_H
