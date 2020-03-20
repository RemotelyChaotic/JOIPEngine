#ifndef THREADEDSYSTEM_H
#define THREADEDSYSTEM_H

#include <QObject>
#include <QPointer>
#include <QThread>
#include <atomic>
#include <functional>
#include <memory>
#include <type_traits>

class CSystemBase : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CSystemBase)

public:
  CSystemBase();
  virtual ~CSystemBase();

  bool IsInitialized() const { return m_bInitialized; }

public slots:
  virtual void Initialize() = 0;
  virtual void Deinitialize() = 0;

protected:
  void SetInitialized(bool bInit) { m_bInitialized = bInit; }

  std::atomic<bool> m_bInitialized;
};

//----------------------------------------------------------------------------------------
//
class CThreadedSystem : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CThreadedSystem)

public:
  CThreadedSystem();
  ~CThreadedSystem();

  std::shared_ptr<CSystemBase> Get() { return m_spSystem; }

  template<typename T,
           typename = std::enable_if<std::is_base_of<CSystemBase, T>::value>>
  void RegisterObject()
  {
    m_spSystem = std::shared_ptr<T>(new T, [](T*){});
    m_spSystem->moveToThread(m_pThread.data());

    connect(m_pThread.data(), &QThread::started, m_spSystem.get(), &CSystemBase::Initialize);
    connect(m_pThread.data(), &QThread::finished, this, &CThreadedSystem::Cleanup, Qt::DirectConnection);
    connect(m_pThread.data(), &QThread::finished, this, &CSystemBase::deleteLater);

    m_pThread->start();
    while (!m_pThread->isRunning())
    {
      thread()->wait(5);
    }
  }

protected slots:
  void Cleanup();

protected:
  std::shared_ptr<CSystemBase> m_spSystem;
  QPointer<QThread>            m_pThread;
};

#endif // THREADEDSYSTEM_H
