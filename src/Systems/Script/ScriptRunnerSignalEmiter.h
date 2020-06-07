#ifndef SCRIPTRUNNERSIGNALEMITER_H
#define SCRIPTRUNNERSIGNALEMITER_H

#include <QColor>
#include <qlogging.h>
#include <QPointer>
#include <QObject>
#include <QString>
#include <memory>

class CScriptRunnerSignalContext;
class CScriptObjectBase;
class QJSEngine;

//----------------------------------------------------------------------------------------
//
class CScriptRunnerSignalEmiter : public QObject
{
  Q_OBJECT

public:
  enum ScriptExecStatus {
    eRunning = 0,
    ePaused,
    eStopped,
  };
  Q_ENUM(ScriptExecStatus)

  CScriptRunnerSignalEmiter();
  CScriptRunnerSignalEmiter(const CScriptRunnerSignalEmiter& other);
  ~CScriptRunnerSignalEmiter();

  virtual std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QPointer<QJSEngine>);

  void Initialize(std::shared_ptr<CScriptRunnerSignalContext> spContext);
  void SetScriptExecutionStatus(ScriptExecStatus status);
  ScriptExecStatus ScriptExecutionStatus();

signals:
  // generic / controll
  void clearStorage();
  void executionError(QString sException, qint32 iLine, QString sStack);
  void interrupt();
  void pauseExecution();
  void resumeExecution();
  void showError(QString sError, QtMsgType type);

protected:
  std::shared_ptr<CScriptRunnerSignalContext> m_spContext;
};

Q_DECLARE_METATYPE(CScriptRunnerSignalEmiter)
Q_DECLARE_METATYPE(QtMsgType)


//----------------------------------------------------------------------------------------
//
class CScriptRunnerSignalContext : public QObject
{
  Q_OBJECT
  friend class CScriptRunnerSignalEmiter;

public:
  CScriptRunnerSignalContext();
  ~CScriptRunnerSignalContext();

public:
  void SetScriptExecutionStatus(CScriptRunnerSignalEmiter::ScriptExecStatus status);
  CScriptRunnerSignalEmiter::ScriptExecStatus ScriptExecutionStatus();

signals:
  // generic / controll
  void clearStorage();
  void executionError(QString sException, qint32 iLine, QString sStack);
  void interrupt();
  void pauseExecution();
  void resumeExecution();
  void showError(QString sError, QtMsgType type);

protected:
  QAtomicInt     m_bScriptExecutionStatus;
};


#endif // SCRIPTRUNNERSIGNALEMITER_H
