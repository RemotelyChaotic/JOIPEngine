#ifndef SCRIPTTIMER_H
#define SCRIPTTIMER_H

#include "ScriptObjectBase.h"
#include <QJSValue>
#include <memory>

class CScriptTimer : public CScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptTimer)

public:
  CScriptTimer(std::shared_ptr<CScriptRunnerSignalEmiter> spEmitter,
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
