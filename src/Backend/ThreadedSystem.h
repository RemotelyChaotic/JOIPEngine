#ifndef THREADEDSYSTEM_H
#define THREADEDSYSTEM_H

#include <QObject>
#include <QThread>
#include <atomic>
#include <functional>
#include <memory>
#include <type_traits>

class CThreadedObject : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CThreadedObject)

public:
  CThreadedObject();
  virtual ~CThreadedObject();

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

  std::shared_ptr<CThreadedObject> Get() { return m_spSystem; }

  template<typename T,
           typename = std::enable_if<std::is_base_of<CThreadedObject, T>::value>>
  void RegisterObject()
  {
    m_spSystem = std::shared_ptr<T>(new T, [](T*){});
    m_spSystem->moveToThread(m_spThread.get());

    connect(m_spThread.get(), &QThread::started, m_spSystem.get(), &CThreadedObject::Initialize);
    connect(m_spThread.get(), &QThread::finished, this, &CThreadedSystem::Cleanup, Qt::DirectConnection);

    m_spThread->start();
    while (!m_spThread->isRunning())
    {
      thread()->wait(5);
    }
  }

protected slots:
  void Cleanup();

protected:
  std::unique_ptr<QThread>         m_spThread;
  std::shared_ptr<CThreadedObject> m_spSystem;
};

#endif // THREADEDSYSTEM_H
