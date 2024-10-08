#ifndef THREADEDSYSTEM_H
#define THREADEDSYSTEM_H

#include <QObject>
#include <QPointer>
#include <QThread>
#include <atomic>
#include <functional>
#include <optional>
#include <memory>
#include <type_traits>

class CSystemBase : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CSystemBase)

public:
  CSystemBase(QThread::Priority prio = QThread::Priority::NormalPriority);
  virtual ~CSystemBase();

  bool IsInitialized() const { return m_bInitialized; }
  QThread::Priority Priority() const { return m_priority; };

public slots:
  virtual void Initialize() = 0;
  virtual void Deinitialize() = 0;

protected:
  void SetInitialized(bool bInit) { m_bInitialized = bInit; }

  std::atomic<bool> m_bInitialized;
  QThread::Priority m_priority;
};

//----------------------------------------------------------------------------------------
//
class CThreadedSystem : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CThreadedSystem)

public:
  CThreadedSystem(const QString& sName);
  ~CThreadedSystem();

  std::shared_ptr<CSystemBase> Get() { return m_spSystem; }

  template<typename T,
           typename ...ARG>
  void RegisterObject(ARG... args)
  {
    m_spSystem = std::shared_ptr<T>(new T(args...), [](T*){});
    m_spSystem->moveToThread(m_pThread.data());

    connect(m_pThread.data(), &QThread::started, m_spSystem.get(), &CSystemBase::Initialize);
    connect(m_pThread.data(), &QThread::finished, this, &CThreadedSystem::Cleanup, Qt::DirectConnection);
    connect(m_pThread.data(), &QThread::finished, this, &CSystemBase::deleteLater);

    m_pThread->start(m_spSystem->Priority());
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
