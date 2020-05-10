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

  virtual std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QPointer<QJSEngine> pEngine);

signals:
  void hideTimer();
  void setTime(qint32 iTimeS);
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
class CScriptTimer : public CScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptTimer)

public:
  CScriptTimer(QPointer<CScriptRunnerSignalEmiter> pEmitter,
               QPointer<QJSEngine> pEngine);
  ~CScriptTimer();

public slots:
  void hide();
  void setTime(qint32 iTimeS);
  void setTimeVisible(bool bVisible);
  void show();
  void start();
  void stop();
  void waitForTimer();
};

#endif // SCRIPTTIMER_H
