#ifndef SCRIPTSTORAGE_H
#define SCRIPTSTORAGE_H

#include <QJSEngine>
#include <QJSValue>
#include <memory>

class CScriptRunnerSignalEmiter;

class CScriptStorage : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptStorage)

public:
  CScriptStorage(std::shared_ptr<CScriptRunnerSignalEmiter> spEmitter,
                 QJSEngine* pEngine);
  ~CScriptStorage();

  void ClearStorage();

public slots:
  QJSValue load(QString sId);
  void store(QString sId, QJSValue value);

private:
  bool CheckIfScriptCanRun();

private slots:
  void SlotClearStorage();

private:
  std::shared_ptr<CScriptRunnerSignalEmiter> m_spSignalEmitter;
  QJSEngine*                                 m_pEngine;
  std::map<QString, QJSValue>                m_storage;
};

#endif // SCRIPTSTORAGE_H
