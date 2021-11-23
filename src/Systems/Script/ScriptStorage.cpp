#include "ScriptStorage.h"
#include "ScriptRunnerSignalEmiter.h"

#include <QDebug>
#include <QEventLoop>
#include <QTimer>
#include <QUuid>

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
  return std::make_shared<CScriptStorage>(this, pEngine);
}

//----------------------------------------------------------------------------------------
//
CScriptStorage::CScriptStorage(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                               QPointer<QJSEngine> pEngine) :
  CJsScriptObjectBase(pEmitter, pEngine)
{
}

CScriptStorage::~CScriptStorage()
{
}

//----------------------------------------------------------------------------------------
//
QJSValue CScriptStorage::load(QString sId)
{
  if (!CheckIfScriptCanRun()) { return QJSValue(); }

  QString sRequestId = QUuid::createUuid().toString();

  auto pSignalEmitter = SignalEmitter<CStorageSignalEmitter>();
  QTimer::singleShot(0, this, [&pSignalEmitter,sId,sRequestId]() {
    emit pSignalEmitter->load(sId, sRequestId);
  });

  // local loop to wait for answer
  QVariant varRetVal = QString();
  QEventLoop loop;
  QMetaObject::Connection quitLoop =
    connect(this, &CScriptStorage::SignalQuitLoop, &loop, &QEventLoop::quit);
  QMetaObject::Connection interruptLoop =
    connect(pSignalEmitter, &CStorageSignalEmitter::interrupt,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
  QMetaObject::Connection showRetValLoop =
    connect(pSignalEmitter, &CStorageSignalEmitter::loadReturnValue,
            this, [this, &varRetVal, sRequestId](QVariant var, QString sRequestIdRet)
  {
    if (sRequestId == sRequestIdRet)
    {
      varRetVal = var;
      varRetVal.detach(); // fixes some crashes with QJSEngine
      emit this->SignalQuitLoop();
    }
    // direct connection to fix cross thread issues with QString content being deleted
  }, Qt::DirectConnection);
  loop.exec();
  loop.disconnect();

  disconnect(quitLoop);
  disconnect(interruptLoop);
  disconnect(showRetValLoop);

  return m_pEngine->toScriptValue(varRetVal);
}

//----------------------------------------------------------------------------------------
//
void CScriptStorage::store(QString sId, QJSValue value)
{
  if (!CheckIfScriptCanRun()) { return; }

  QVariant var = value.toVariant();
  emit SignalEmitter<CStorageSignalEmitter>()->store(sId, var);
}

//----------------------------------------------------------------------------------------
//
void CScriptStorage::Cleanup_Impl()
{
}
