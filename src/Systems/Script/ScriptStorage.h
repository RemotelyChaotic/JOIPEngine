#ifndef SCRIPTSTORAGE_H
#define SCRIPTSTORAGE_H

#include "CommonScriptHelpers.h"
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

signals:
  void clear();
  void load(QString sId, QString sRequestId, QString sContext);
  void loadPersistent(QString sId);
  void loadReturnValue(QJSValue value, QString sRequestId);
  void removeData(QString sId, QString sContext);
  void store(QString sId, QVariant value, QString sContext);
  void storePersistent(QString sId);
  void valueStored(QString sId, QJSValue value);

  void ValueStoredPrivate(QString sId, QVariant value);

protected:
  std::shared_ptr<CScriptCommunicator>
  CreateCommunicatorImpl(std::shared_ptr<CScriptRunnerSignalEmiterAccessor> spAccessor) override;
};

//----------------------------------------------------------------------------------------
//
class CStorageScriptCommunicator : public CScriptCommunicator
{
  public:
  CStorageScriptCommunicator(const std::weak_ptr<CScriptRunnerSignalEmiterAccessor>& spEmitter);
  ~CStorageScriptCommunicator() override;

  CScriptObjectBase* CreateNewScriptObject(QPointer<QJSEngine> pEngine) override;
  CScriptObjectBase* CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser) override;
  CScriptObjectBase* CreateNewScriptObject(QtLua::State* pState) override;
  CScriptObjectBase* CreateNewSequenceObject() override;
};

//----------------------------------------------------------------------------------------
//
class CScriptStorageBase : public CJsScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptStorageBase)

public:
  CScriptStorageBase(std::weak_ptr<CScriptCommunicator> pCommunicator,
                     QPointer<QJSEngine> pEngine);
  CScriptStorageBase(std::weak_ptr<CScriptCommunicator> pCommunicator,
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
  void ValueChanged(QString sId, QVariant value);

  std::map<QString, script::tCallbackValue> m_callbacks;

private:
  std::shared_ptr<std::function<void()>> m_spStop;
};

//----------------------------------------------------------------------------------------
//
class CScriptStorageJs : public CScriptStorageBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptStorageJs)
public:
  CScriptStorageJs(std::weak_ptr<CScriptCommunicator> pCommunicator,
                   QPointer<QJSEngine> pEngine);
  ~CScriptStorageJs();

public slots:
  QVariant load(QString sId);
  QVariant loadAchievement(QString sId);
  void store(QString sId, QVariant value);
  void storeAchievement(QString sId, QVariant value);

  void registerChangeCallback(const QString& sId, QJSValue callback);
};

//----------------------------------------------------------------------------------------
//
class CScriptStorageLua : public CScriptStorageBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptStorageLua)

public:
  CScriptStorageLua(std::weak_ptr<CScriptCommunicator> pCommunicator,
                    QtLua::State* pState);
  ~CScriptStorageLua();

public slots:
  QVariant load(QString sId);
  QVariant loadAchievement(QString sId);
  void store(QString sId, QVariant value);
  void storeAchievement(QString sId, QVariant value);

  void registerChangeCallback(const QString& sId, QVariant callback);
};

#endif // SCRIPTSTORAGE_H
