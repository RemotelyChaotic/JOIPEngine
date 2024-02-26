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

  std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QPointer<QJSEngine> pEngine) override;
  std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QtLua::State* pState) override;
  std::shared_ptr<CScriptObjectBase> CreateNewSequenceObject() override;

signals:
  void skippableWait(qint32 iTimeS);
  void waitSkipped();
};
Q_DECLARE_METATYPE(CThreadSignalEmitter)

//----------------------------------------------------------------------------------------
//
class CScriptThread : public CJsScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptThread)

public:
  CScriptThread(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                QPointer<QJSEngine> pEngine);
  CScriptThread(QPointer<CScriptRunnerSignalEmiter> pEmitter,
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

private:
  QString GetResourceName(const QVariant& resource, const QString& sMethod,
                          bool bStringCanBeId = false, bool* pbOk = nullptr);

  std::weak_ptr<CDatabaseManager>  m_wpDbManager;
};

#endif // SCRIPTTHREAD_H
