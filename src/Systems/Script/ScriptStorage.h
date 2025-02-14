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

  std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QPointer<QJSEngine> pEngine) override;
  std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QtLua::State* pState) override;
  std::shared_ptr<CScriptObjectBase> CreateNewSequenceObject() override;

signals:
  void clear();
  void load(QString sId, QString sRequestId, QString sContext);
  void loadPersistent(QString sId);
  void loadReturnValue(QJSValue value, QString sRequestId);
  void removeData(QString sId, QString sContext);
  void store(QString sId, QVariant value, QString sContext);
  void storePersistent(QString sId);
};
Q_DECLARE_METATYPE(CStorageSignalEmitter)

//----------------------------------------------------------------------------------------
//
class CScriptStorageBase : public CJsScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptStorageBase)

public:
  CScriptStorageBase(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                     QPointer<QJSEngine> pEngine);
  CScriptStorageBase(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                     QtLua::State* pState);
  ~CScriptStorageBase();

public slots:
  void loadPersistent(QString sId);
  void removeData(QString sId);
  void storePersistent(QString sId);

signals:
  void SignalQuitLoop();

protected:
  void Cleanup_Impl() override;
  QVariant LoadImpl(QString sId, QString sContext);
  void StoreImpl(QString sId, QVariant value, QString sContext);
};

//----------------------------------------------------------------------------------------
//
class CScriptStorageJs : public CScriptStorageBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptStorageJs)
public:
  CScriptStorageJs(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                   QPointer<QJSEngine> pEngine);
  ~CScriptStorageJs();

public slots:
  QVariant load(QString sId);
  QVariant loadAchievement(QString sId);
  void store(QString sId, QVariant value);
  void storeAchievement(QString sId, QVariant value);

};

//----------------------------------------------------------------------------------------
//
class CScriptStorageLua : public CScriptStorageBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptStorageLua)

public:
  CScriptStorageLua(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                    QtLua::State* pState);
  ~CScriptStorageLua();

public slots:
  QVariant load(QString sId);
  QVariant loadAchievement(QString sId);
  void store(QString sId, QVariant value);
  void storeAchievement(QString sId, QVariant value);
};

#endif // SCRIPTSTORAGE_H
