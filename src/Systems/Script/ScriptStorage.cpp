#include "ScriptStorage.h"
#include "ScriptRunnerSignalEmiter.h"
#include <QDebug>

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
  CScriptObjectBase(pEmitter, pEngine),
  m_storage()
{
  connect(m_pSignalEmitter, &CScriptRunnerSignalEmiter::clearStorage,
          this, &CScriptStorage::SlotClearStorage, Qt::QueuedConnection);
}

CScriptStorage::~CScriptStorage()
{
  SlotClearStorage();
}

//----------------------------------------------------------------------------------------
//
QJSValue CScriptStorage::load(QString sId)
{
  if (!CheckIfScriptCanRun()) { return QJSValue(); }

  auto it = m_storage.find(sId);
  if (m_storage.end() != it)
  {
    return it->second;
  }
  return QJSValue();
}

//----------------------------------------------------------------------------------------
//
void CScriptStorage::store(QString sId, QJSValue value)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (value.isNull() || value.isUndefined())
  {
    QString sError = tr("Storing 'null' or 'undefined' object as '%1'.");
    qWarning() << sError.arg(sId);
    emit m_pSignalEmitter->showError(sError.arg(sId), QtMsgType::QtWarningMsg);
  }

  m_storage[sId] = value;
}

//----------------------------------------------------------------------------------------
//
void CScriptStorage::Cleanup_Impl()
{
  SlotClearStorage();
}

//----------------------------------------------------------------------------------------
//
void CScriptStorage::SlotClearStorage()
{
  m_storage.clear();
}
