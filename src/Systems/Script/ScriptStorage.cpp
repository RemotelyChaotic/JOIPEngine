#include "ScriptStorage.h"
#include "CommonScriptHelpers.h"
#include "ScriptRunnerSignalEmiter.h"

#include "Systems/SaveData.h"

#include <QDebug>
#include <QEventLoop>
#include <QJSValue>
#include <QTimer>
#include <QUuid>

#include <functional>

CStorageSignalEmitter::CStorageSignalEmitter() :
  CScriptRunnerSignalEmiter()
{

}
CStorageSignalEmitter::~CStorageSignalEmitter()
{

}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CScriptObjectBase> CStorageSignalEmitter::CreateNewScriptObject(QPointer<QJSEngine> pEngine)
{
  return std::make_shared<CScriptStorageJs>(this, pEngine);
}
std::shared_ptr<CScriptObjectBase> CStorageSignalEmitter::CreateNewScriptObject(QtLua::State* pState)
{
  return std::make_shared<CScriptStorageLua>(this, pState);
}
std::shared_ptr<CScriptObjectBase> CStorageSignalEmitter::CreateNewSequenceObject()
{
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
CScriptStorageBase::CScriptStorageBase(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                                       QPointer<QJSEngine> pEngine) :
  CJsScriptObjectBase(pEmitter, pEngine)
{
}
CScriptStorageBase::CScriptStorageBase(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                                       QtLua::State* pState) :
  CJsScriptObjectBase(pEmitter, pState)
{
}

CScriptStorageBase::~CScriptStorageBase()
{
}

//----------------------------------------------------------------------------------------
//
void CScriptStorageBase::loadPersistent(QString sId)
{
  emit SignalEmitter<CStorageSignalEmitter>()->loadPersistent(sId);
}

//----------------------------------------------------------------------------------------
//
void CScriptStorageBase::removeData(QString sId)
{
  emit SignalEmitter<CStorageSignalEmitter>()->removeData(sId, QString());
}

//----------------------------------------------------------------------------------------
//
void CScriptStorageBase::storePersistent(QString sId)
{
  emit SignalEmitter<CStorageSignalEmitter>()->storePersistent(sId);
}

//----------------------------------------------------------------------------------------
//
void CScriptStorageBase::Cleanup_Impl()
{
}

//----------------------------------------------------------------------------------------
//
QVariant CScriptStorageBase::LoadImpl(QString sId, QString sContext)
{
  QString sRequestId = QUuid::createUuid().toString();

  auto pSignalEmitter = SignalEmitter<CStorageSignalEmitter>();
  QTimer::singleShot(0, this, [&pSignalEmitter,sId,sContext,sRequestId]() {
    emit pSignalEmitter->load(sId, sRequestId,sContext);
  });

  // local loop to wait for answer
  QVariant varRetVal = QString();
  QEventLoop loop;
  QMetaObject::Connection quitLoop =
    connect(this, &CScriptStorageBase::SignalQuitLoop, &loop, &QEventLoop::quit);
  QMetaObject::Connection interruptLoop =
    connect(pSignalEmitter, &CStorageSignalEmitter::interrupt,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
  QMetaObject::Connection interruptThisLoop =
    connect(this, &CScriptObjectBase::SignalInterruptExecution,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
  QMetaObject::Connection showRetValLoop =
    connect(pSignalEmitter, &CStorageSignalEmitter::loadReturnValue,
            this, [this, &varRetVal, sRequestId](QJSValue var, QString sRequestIdRet)
  {
    if (sRequestId == sRequestIdRet)
    {
      varRetVal = var.toVariant();
      varRetVal.detach(); // fixes some crashes with QJSEngine
      emit this->SignalQuitLoop();
    }
    // direct connection to fix cross thread issues with QString content being deleted
  }, Qt::DirectConnection);
  loop.exec();
  loop.disconnect();

  disconnect(quitLoop);
  disconnect(interruptLoop);
  disconnect(interruptThisLoop);
  disconnect(showRetValLoop);

  return varRetVal;
}

//----------------------------------------------------------------------------------------
//
void CScriptStorageBase::StoreImpl(QString sId, QVariant value, QString sContext)
{
  emit SignalEmitter<CStorageSignalEmitter>()->store(sId, value, sContext);
}

//----------------------------------------------------------------------------------------
//
CScriptStorageJs::CScriptStorageJs(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                                   QPointer<QJSEngine> pEngine) :
  CScriptStorageBase(pEmitter, pEngine)
{
}
CScriptStorageJs::~CScriptStorageJs()
{
}

//----------------------------------------------------------------------------------------
//
QVariant CScriptStorageJs::load(QString sId)
{
  if (!CheckIfScriptCanRun()) { return QVariant(); }
  return LoadImpl(sId, QString());
}

//----------------------------------------------------------------------------------------
//
QVariant CScriptStorageJs::loadAchievement(QString sId)
{
  if (!CheckIfScriptCanRun()) { return QVariant(); }
  return LoadImpl(sId, save_data::c_sFileAchievements);
}

//----------------------------------------------------------------------------------------
//
void CScriptStorageJs::store(QString sId, QVariant value)
{
  if (!CheckIfScriptCanRun()) { return; }

  QVariant valueToWrite = value;
  QJSValue valFromJS = value.value<QJSValue>();
  if (valFromJS.isArray())
  {
    valueToWrite = valFromJS.toVariant();
  }
  else if (valFromJS.isObject())
  {
    valueToWrite = valFromJS.toVariant();
  }

  StoreImpl(sId, valueToWrite, QString());
}

//----------------------------------------------------------------------------------------
//
void CScriptStorageJs::storeAchievement(QString sId, QVariant value)
{
  if (!CheckIfScriptCanRun()) { return ; }

  QVariant valueToWrite = value;
  QJSValue valFromJS = value.value<QJSValue>();
  if (valFromJS.isArray())
  {
    valueToWrite = valFromJS.toVariant();
  }
  else if (valFromJS.isObject())
  {
    valueToWrite = valFromJS.toVariant();
  }

  StoreImpl(sId, valueToWrite, save_data::c_sFileAchievements);
}

//----------------------------------------------------------------------------------------
//
CScriptStorageLua::CScriptStorageLua(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                                     QtLua::State* pState) :
  CScriptStorageBase(pEmitter, pState)
{
}
CScriptStorageLua::~CScriptStorageLua()
{
}

//----------------------------------------------------------------------------------------
//
QVariant CScriptStorageLua::load(QString sId)
{
  if (!CheckIfScriptCanRun()) { return QVariant(); }
  return LoadImpl(sId, QString());
}

//----------------------------------------------------------------------------------------
//
QVariant CScriptStorageLua::loadAchievement(QString sId)
{
  if (!CheckIfScriptCanRun()) { return QVariant(); }
  return LoadImpl(sId, save_data::c_sFileAchievements);
}

//----------------------------------------------------------------------------------------
//
void CScriptStorageLua::store(QString sId, QVariant value)
{
  if (!CheckIfScriptCanRun()) { return; }
  StoreImpl(sId, script::ConvertLuaVariant(value), QString());
}

//----------------------------------------------------------------------------------------
//
void CScriptStorageLua::storeAchievement(QString sId, QVariant value)
{
  if (!CheckIfScriptCanRun()) { return ; }
  StoreImpl(sId, script::ConvertLuaVariant(value), save_data::c_sFileAchievements);
}
