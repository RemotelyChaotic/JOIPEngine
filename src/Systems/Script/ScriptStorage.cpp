#include "ScriptStorage.h"
#include "ScriptRunnerSignalEmiter.h"
#include <QDebug>

CScriptStorage::CScriptStorage(std::shared_ptr<CScriptRunnerSignalEmiter> spEmitter,
                               QPointer<QJSEngine> pEngine) :
  CScriptObjectBase(spEmitter, pEngine),
  m_storage()
{
  connect(m_spSignalEmitter.get(), &CScriptRunnerSignalEmiter::SignalClearStorage,
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
    return m_pEngine->toScriptValue(it->second);
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
    emit m_spSignalEmitter->SignalShowError(sError.arg(sId), QtMsgType::QtWarningMsg);
  }

  m_storage.insert({sId, value});
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
