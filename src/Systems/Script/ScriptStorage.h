#ifndef SCRIPTSTORAGE_H
#define SCRIPTSTORAGE_H

#include "ScriptObjectBase.h"
#include "ScriptRunnerSignalEmiter.h"
#include <QJSValue>

class CScriptRunnerSignalEmiter;


class CStorageSignalEmitter : public CScriptRunnerSignalEmiter
{
  Q_OBJECT
public:
  CStorageSignalEmitter();
  ~CStorageSignalEmitter();

  virtual std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QPointer<QJSEngine> pEngine);

signals:
  void clear();
  void load(QString sId, QString sRequestId);
  void loadReturnValue(QJSValue value, QString sRequestId);
  void store(QString sId, QVariant value);
};
Q_DECLARE_METATYPE(CStorageSignalEmitter)

//----------------------------------------------------------------------------------------
//
class CScriptStorage : public CJsScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptStorage)

public:
  CScriptStorage(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                 QPointer<QJSEngine> pEngine);
  ~CScriptStorage();

public slots:
  QJSValue load(QString sId);
  void store(QString sId, QJSValue value);

signals:
  void SignalQuitLoop();

protected:
  void Cleanup_Impl() override;
};

#endif // SCRIPTSTORAGE_H
