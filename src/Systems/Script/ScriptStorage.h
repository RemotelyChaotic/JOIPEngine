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
};
Q_DECLARE_METATYPE(CStorageSignalEmitter)

//----------------------------------------------------------------------------------------
//
class CScriptStorage : public CScriptObjectBase
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

protected:
  void Cleanup_Impl() override;

private slots:
  void SlotClearStorage();

private:
  std::map<QString, QJSValue>                m_storage;
};

#endif // SCRIPTSTORAGE_H
