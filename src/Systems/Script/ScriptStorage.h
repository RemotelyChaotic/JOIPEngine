#ifndef SCRIPTSTORAGE_H
#define SCRIPTSTORAGE_H

#include "ScriptObjectBase.h"
#include <QJSValue>

class CScriptRunnerSignalEmiter;

class CScriptStorage : public CScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptStorage)

public:
  CScriptStorage(std::shared_ptr<CScriptRunnerSignalEmiter> spEmitter,
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
