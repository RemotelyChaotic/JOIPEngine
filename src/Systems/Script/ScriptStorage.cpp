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
std::shared_ptr<CScriptCommunicator>
CStorageSignalEmitter::CreateCommunicatorImpl(std::shared_ptr<CScriptRunnerSignalEmiterAccessor> spAccessor)
{
  return std::make_shared<CStorageScriptCommunicator>(spAccessor);
}

//----------------------------------------------------------------------------------------
//
CStorageScriptCommunicator::CStorageScriptCommunicator(
  const std::weak_ptr<CScriptRunnerSignalEmiterAccessor>& spEmitter) :
  CScriptCommunicator(spEmitter)
{}
CStorageScriptCommunicator::~CStorageScriptCommunicator() = default;

//----------------------------------------------------------------------------------------
//
CScriptObjectBase* CStorageScriptCommunicator::CreateNewScriptObject(QPointer<QJSEngine> pEngine)
{
  return new CScriptStorageJs(weak_from_this(), pEngine);
}
CScriptObjectBase* CStorageScriptCommunicator::CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser)
{
  Q_UNUSED(pParser)
  return nullptr;
}
CScriptObjectBase* CStorageScriptCommunicator::CreateNewScriptObject(QtLua::State* pState)
{
  return new CScriptStorageLua(weak_from_this(), pState);
}
CScriptObjectBase* CStorageScriptCommunicator::CreateNewSequenceObject()
{
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
CScriptStorageBase::CScriptStorageBase(std::weak_ptr<CScriptCommunicator> pCommunicator,
                                       QPointer<QJSEngine> pEngine) :
  CJsScriptObjectBase(pCommunicator, pEngine)
{
  m_spStop = std::make_shared<std::function<void()>>([this]() {
    emit SignalQuitLoop();
  });
  if (auto spComm = pCommunicator.lock())
  {
    spComm->RegisterStopCallback(m_spStop);
  }
}
CScriptStorageBase::CScriptStorageBase(std::weak_ptr<CScriptCommunicator> pCommunicator,
                                       QtLua::State* pState) :
  CJsScriptObjectBase(pCommunicator, pState)
{
  m_spStop = std::make_shared<std::function<void()>>([this]() {
    emit SignalQuitLoop();
  });
  if (auto spComm = pCommunicator.lock())
  {
    spComm->RegisterStopCallback(m_spStop);
  }
}

CScriptStorageBase::~CScriptStorageBase()
{
  if (auto spComm = m_wpCommunicator.lock())
  {
    spComm->RemoveStopCallback(m_spStop);
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptStorageBase::loadPersistent(QString sId)
{
  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CStorageSignalEmitter>())
    {
      emit spSignalEmitter->loadPersistent(sId);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptStorageBase::removeData(QString sId)
{
  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CStorageSignalEmitter>())
    {
      spSignalEmitter->removeData(sId, QString());
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptStorageBase::storePersistent(QString sId)
{
  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CStorageSignalEmitter>())
    {
      spSignalEmitter->storePersistent(sId);
    }
  }
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

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CStorageSignalEmitter>())
    {
      QTimer::singleShot(0, this, [this,sId,sContext,sRequestId]() {
        if (auto spComm = m_wpCommunicator.lock())
        {
          if (auto spSignalEmitter = spComm->LockedEmitter<CStorageSignalEmitter>())
          {
            emit spSignalEmitter->load(sId, sRequestId, sContext);
          }
        }
      });

      // local loop to wait for answer
      QPointer<CScriptStorageBase> pThis(this);
      std::shared_ptr<QVariant> spReturnValue = std::make_shared<QVariant>();
      QEventLoop loop;
      QMetaObject::Connection quitLoop =
        connect(this, &CScriptStorageBase::SignalQuitLoop, &loop, &QEventLoop::quit);
      QMetaObject::Connection interruptThisLoop =
        connect(this, &CScriptObjectBase::SignalInterruptExecution,
                &loop, &QEventLoop::quit, Qt::QueuedConnection);
      QMetaObject::Connection showRetValLoop =
        connect(spSignalEmitter.Get(), &CStorageSignalEmitter::loadReturnValue,
                &loop, [&loop, spReturnValue, sRequestId](QJSValue var, QString sRequestIdRet)
      {
        if (sRequestId == sRequestIdRet)
        {
          *spReturnValue = var.toVariant();
          spReturnValue->detach(); // fixes some crashes with QJSEngine
          bool bOk = QMetaObject::invokeMethod(&loop, "quit", Qt::QueuedConnection);
          assert(bOk); Q_UNUSED(bOk)
        }
        // direct connection to fix cross thread issues with QString content being deleted
      }, Qt::DirectConnection);
      loop.exec();
      loop.disconnect();

      if (nullptr != pThis)
      {
        disconnect(quitLoop);
        disconnect(interruptThisLoop);
        disconnect(showRetValLoop);
      }

      return *spReturnValue;
    }
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
void CScriptStorageBase::StoreImpl(QString sId, QVariant value, QString sContext)
{
  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CStorageSignalEmitter>())
    {
      spSignalEmitter->store(sId, value, sContext);
    }
  }
}

//----------------------------------------------------------------------------------------
//
CScriptStorageJs::CScriptStorageJs(std::weak_ptr<CScriptCommunicator> pCommunicator,
                                   QPointer<QJSEngine> pEngine) :
  CScriptStorageBase(pCommunicator, pEngine)
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
CScriptStorageLua::CScriptStorageLua(std::weak_ptr<CScriptCommunicator> pCommunicator,
                                     QtLua::State* pState) :
  CScriptStorageBase(pCommunicator, pState)
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
