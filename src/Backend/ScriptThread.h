#ifndef SCRIPTTHREAD_H
#define SCRIPTTHREAD_H

#include <QJSEngine>
#include <QJSValue>
#include <memory>

class CScriptRunnerSignalEmiter;

class CScriptThread : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptThread)

public:
  CScriptThread(std::shared_ptr<CScriptRunnerSignalEmiter> spEmitter,
                QJSEngine* pEngine);
  ~CScriptThread();

public slots:
  void sleep(qint32 iTimeS, QJSValue bSkippable);

private:
  bool CheckIfScriptCanRun();

  std::shared_ptr<CScriptRunnerSignalEmiter> m_spSignalEmitter;
  QJSEngine*               m_pEngine;
};

#endif // SCRIPTTHREAD_H
