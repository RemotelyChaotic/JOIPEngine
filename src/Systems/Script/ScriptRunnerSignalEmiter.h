#ifndef SCRIPTRUNNERSIGNALEMITER_H
#define SCRIPTRUNNERSIGNALEMITER_H

#include <QColor>
#include <qlogging.h>
#include <QJSValue>
#include <QMutex>
#include <QPointer>
#include <QObject>
#include <QString>
#include <memory>

class CJsonInstructionSetParser;
class CScriptCommunicator;
class CScriptRunnerSignalContext;
class CScriptRunnerSignalEmiter;
class CScriptObjectBase;
class QJSEngine;
namespace QtLua {
  class State;
}

class CScriptRunnerSignalEmiterAccessor
{
  friend class CScriptRunnerSignalEmiter;

public:
  CScriptRunnerSignalEmiterAccessor(CScriptRunnerSignalEmiter* pParent);

  CScriptRunnerSignalEmiter* Get() const;
  void Lock();
  void Unlock();

private:
  CScriptRunnerSignalEmiter* m_pParent;
  QMutex                     m_mutex;
};

//----------------------------------------------------------------------------------------
//
template<typename T,
         typename std::enable_if_t<std::is_base_of_v<CScriptRunnerSignalEmiter, T>, bool> = 0>
class CLockedEmitter
{
public:
  CLockedEmitter(std::weak_ptr<CScriptRunnerSignalEmiterAccessor> wpSignalEmitterAccess) :
    m_spSignalEmitterAccess(wpSignalEmitterAccess.lock())
  {
    if (nullptr != m_spSignalEmitterAccess && nullptr != m_spSignalEmitterAccess->Get())
    {
      m_spSignalEmitterAccess->Lock();
    }
  }
  ~CLockedEmitter()
  {
    if (nullptr != m_spSignalEmitterAccess && nullptr != m_spSignalEmitterAccess->Get())
    {
      m_spSignalEmitterAccess->Unlock();
    }
  }

  T* Get() const
  {
    return nullptr != m_spSignalEmitterAccess ?
               dynamic_cast<T*>(m_spSignalEmitterAccess->Get()) : nullptr;
  }

  operator bool() const { return nullptr != m_spSignalEmitterAccess &&
                                 nullptr != m_spSignalEmitterAccess->Get() ?
                                    true : false; }
  const T* operator ->() const { return dynamic_cast<T*>(m_spSignalEmitterAccess->Get()); }
  T* operator ->() { return dynamic_cast<T*>(m_spSignalEmitterAccess->Get()); }

private:
  std::shared_ptr<CScriptRunnerSignalEmiterAccessor> m_spSignalEmitterAccess;
};

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
  ~CScriptRunnerSignalEmiter();

  std::weak_ptr<CScriptCommunicator> Communicator() const;
  void Initialize(std::shared_ptr<CScriptRunnerSignalContext> spContext);
  ScriptExecStatus ScriptExecutionStatus() const;

signals:
  // generic / controll
  void executionError(QString sException, qint32 iLine, QString sStack);
  void requestValue(const QString sValue, const QString sId);
  void requestValueRet(QJSValue sValue, const QString sId);
  void showError(QString sError, QtMsgType type);

protected:
  std::shared_ptr<CScriptCommunicator> CreateCommunicator();
  virtual std::shared_ptr<CScriptCommunicator>
  CreateCommunicatorImpl(std::shared_ptr<CScriptRunnerSignalEmiterAccessor>) { return nullptr; }

  std::shared_ptr<CScriptRunnerSignalEmiterAccessor> m_spAccessor;
  std::shared_ptr<CScriptCommunicator>               m_spCommunicator;
  std::weak_ptr<CScriptRunnerSignalContext>          m_wpContext;
};

Q_DECLARE_METATYPE(CScriptRunnerSignalEmiter*)
Q_DECLARE_METATYPE(QtMsgType)

//----------------------------------------------------------------------------------------
//
class CScriptCommunicator : public std::enable_shared_from_this<CScriptCommunicator>
{
  friend class CScriptRunnerSignalEmiter;

public:
  CScriptCommunicator(const std::weak_ptr<CScriptRunnerSignalEmiterAccessor>& spEmitter);
  virtual ~CScriptCommunicator();

  virtual CScriptObjectBase* CreateNewScriptObject(QPointer<QJSEngine>) = 0;
  virtual CScriptObjectBase* CreateNewScriptObject(QPointer<CJsonInstructionSetParser>) = 0;
  virtual CScriptObjectBase* CreateNewScriptObject(QtLua::State*) = 0;
  virtual CScriptObjectBase* CreateNewSequenceObject() = 0;

  void RegisterResumeCallback(const std::shared_ptr<std::function<void()>>& spFn);
  void RegisterPauseCallback(const std::shared_ptr<std::function<void()>>& spFn);
  void RegisterStopCallback(const std::shared_ptr<std::function<void()>>& spFn);
  void RemoveResumeCallback(const std::shared_ptr<std::function<void()>>& spFn);
  void RemovePauseCallback(const std::shared_ptr<std::function<void()>>& spFn);
  void RemoveStopCallback(const std::shared_ptr<std::function<void()>>& spFn);

  CScriptRunnerSignalEmiter::ScriptExecStatus ScriptExecutionStatus() const;
  template <typename T> CLockedEmitter<T> LockedEmitter() const
  {
    QMutexLocker l(&m_mutex);
    return CLockedEmitter<T>(m_wpSignalEmitterAccess);
  }

protected:
  void RegisterCallback(std::vector<std::weak_ptr<std::function<void()>>>* vspfnCallbacks,
                        const std::shared_ptr<std::function<void()>>& spFn);
  void RemoveCallback(std::vector<std::weak_ptr<std::function<void()>>>* vspfnCallbacks,
                      const std::shared_ptr<std::function<void()>>& spFn);

  mutable QMutex                                    m_mutex;
  std::weak_ptr<CScriptRunnerSignalEmiterAccessor>  m_wpSignalEmitterAccess;
  std::vector<std::weak_ptr<std::function<void()>>> m_vspfnResumeCallback;
  std::vector<std::weak_ptr<std::function<void()>>> m_vspfnPauseCallback;
  std::vector<std::weak_ptr<std::function<void()>>> m_vspfnStopCallback;
};

Q_DECLARE_METATYPE(std::weak_ptr<CScriptCommunicator>)

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
  CScriptRunnerSignalEmiter::ScriptExecStatus ScriptExecutionStatus() const;

signals:
  // running state
  void startExecution();
  void pauseExecution();
  void resumeExecution();
  void stopExecution();

  // generic / controll
  void executionError(QString sException, qint32 iLine, QString sStack);
  void showError(QString sError, QtMsgType type);

protected:
  QAtomicInt     m_bScriptExecutionStatus;
};


#endif // SCRIPTRUNNERSIGNALEMITER_H
