#ifndef SCRIPTTIMER_H
#define SCRIPTTIMER_H

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

  std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QPointer<QJSEngine> pEngine) override;
  std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser) override;

signals:
  void hideTimer();
  void setTime(double dTimeS);
  void setTimeVisible(bool bVisible);
  void showTimer();
  void startTimer();
  void stopTimer();
  void waitForTimer();
  void timerFinished();
};
Q_DECLARE_METATYPE(CTimerSignalEmitter)

//----------------------------------------------------------------------------------------
//
class CScriptTimer : public CJsScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptTimer)

public:
  CScriptTimer(QPointer<CScriptRunnerSignalEmiter> pEmitter,
               QPointer<QJSEngine> pEngine);
  ~CScriptTimer();

public slots:
  void hide();
  void setTime(double dTimeS);
  void setTimeVisible(bool bVisible);
  void show();
  void start();
  void stop();
  void waitForTimer();
};

//----------------------------------------------------------------------------------------
//
class CCommandEosTimer;
class CEosScriptTimer : public CEosScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CEosScriptTimer)

public:
  CEosScriptTimer(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                  QPointer<CJsonInstructionSetParser> pParser);
  ~CEosScriptTimer();

  void hide();
  void setTime(double dTimeS);
  void setTimeVisible(bool bVisible);
  void sleep(qint64 iTimeMs);
  void show();
  void start();
  void waitForTimer();

private:
  std::shared_ptr<CCommandEosTimer> m_spCommandTimer;
};

#endif // SCRIPTTIMER_H
