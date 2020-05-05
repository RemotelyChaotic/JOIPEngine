#ifndef SCRIPTTHREAD_H
#define SCRIPTTHREAD_H

#include "ScriptObjectBase.h"
#include <QJSValue>
#include <memory>

class CScriptThread : public CScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptThread)

public:
  CScriptThread(std::shared_ptr<CScriptRunnerSignalEmiter> spEmitter,
                QPointer<QJSEngine> pEngine);
  ~CScriptThread();

public slots:
  void sleep(qint32 iTimeS);
  void sleep(qint32 iTimeS, QJSValue bSkippable);

};

#endif // SCRIPTTHREAD_H
