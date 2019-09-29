#ifndef SCRIPTTIMER_H
#define SCRIPTTIMER_H

#include <QJSEngine>
#include <QJSValue>
#include <memory>

class CScriptRunnerSignalEmiter;

class CScriptTimer : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptTimer)

public:
  CScriptTimer(std::shared_ptr<CScriptRunnerSignalEmiter> spEmitter, QJSEngine* pEngine);
  ~CScriptTimer();

public slots:
  void hide();
  void setTime(qint32 iTimeS);
  void setTimeVisible(bool bVisible);
  void show();
  void start();
  void stop();
  void waitForTimer();

private:
  bool CheckIfScriptCanRun();

  std::shared_ptr<CScriptRunnerSignalEmiter> m_spSignalEmitter;
  QJSEngine*               m_pEngine;
};

#endif // SCRIPTTIMER_H
