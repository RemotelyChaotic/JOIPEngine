#include "ScriptStorage.h"
#include "ScriptRunnerSignalEmiter.h"
#include <QDebug>

CScriptStorage::CScriptStorage(std::shared_ptr<CScriptRunnerSignalEmiter> spEmitter,
                               QJSEngine* pEngine) :
  QObject(),
  m_spSignalEmitter(spEmitter),
  m_pEngine(pEngine),
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
void CScriptStorage::ClearStorage()
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
bool CScriptStorage::CheckIfScriptCanRun()
{
  if (m_spSignalEmitter->ScriptExecutionStatus()._to_integral() == EScriptExecutionStatus::eStopped)
  {
    QJSValue val = m_pEngine->evaluate("f();"); //undefined function -> create error
    Q_UNUSED(val);
    return false;
  }
  else
  {
    return true;
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptStorage::SlotClearStorage()
{
  m_storage.clear();
}
