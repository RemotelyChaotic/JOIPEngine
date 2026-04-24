#ifndef LOCKEDTYPE_H
#define LOCKEDTYPE_H

#include <QDebug>
#include <QMutex>

#include <cassert>
#include <memory>
#include <type_traits>

template <typename T>
class LockedTypeAutoLocker;

template<typename T>
class LockedType
{
public:
  LockedType() :
      m_mutex(QMutex::Recursive),
      m_spType(std::make_unique<T>())
  {
  }
  LockedType(const LockedType&) = delete;
  template <typename TArg1, typename... TArgs,
            typename std::enable_if_t<std::is_constructible_v<T, TArg1, TArgs...> &&
                                      std::is_same<LockedType<T>,
                                                   typename std::decay<TArg1>::type>::value, int> = 0>
  explicit LockedType(TArg1&& arg1, TArgs&&... args) :
      m_mutex(QMutex::Recursive),
      m_spType(std::make_unique<T>(std::forward<TArg1>(arg1), std::forward<TArgs>(args)...))
  {}
  ~LockedType()
  {
#ifndef NDEBUG
    if (m_mutex.tryLock())
    {
      m_mutex.unlock();
    }
    else
    {
      qWarning() << "Type was locked.";
      assert(false && "Type was locked.");
    }
#endif
  }

  LockedType& operator =(const LockedType&) = delete;

  LockedTypeAutoLocker<T> Locked() const
  {
    return LockedTypeAutoLocker<T>{m_spType.get(), &m_mutex};
  }
  void Reset(T* pReplace)
  {
    QMutexLocker l(&m_mutex);
    m_spType.reset(pReplace);
  }

private:
  mutable QMutex     m_mutex;
  std::unique_ptr<T> m_spType;
};

//----------------------------------------------------------------------------------------
//
template<typename T>
class LockedTypeAutoLocker
{
public:
  LockedTypeAutoLocker() = delete;
  LockedTypeAutoLocker(T* pWrapped, QMutex* pMutex) :
    m_pMutex(pMutex),
    m_pTypeWrapped(pWrapped)
  {
    assert(nullptr != m_pMutex);
    assert(nullptr != m_pTypeWrapped);
    if (nullptr != m_pMutex)
    {
      Lock();
    }
  }
  LockedTypeAutoLocker(const LockedTypeAutoLocker&) = delete;
  LockedTypeAutoLocker& operator =(const LockedTypeAutoLocker&) = delete;
  ~LockedTypeAutoLocker()
  {
    if (nullptr != m_pMutex)
    {
      Unlock();
    }
  }

  void Lock()
  {
    if (Q_LIKELY(nullptr != m_pMutex))
    {
      m_pMutex->lock();
    }
  }

  void Unlock()
  {
    if (Q_LIKELY(nullptr != m_pMutex))
    {
      m_pMutex->unlock();
    }
  }

  T* Get() const
  {
    return nullptr != m_pTypeWrapped ? m_pTypeWrapped : nullptr;
  }

  operator bool() const { return nullptr != m_pTypeWrapped ? true : false; }
  const T* operator ->() const { return m_pTypeWrapped; }
  T* operator ->() { return m_pTypeWrapped; }

private:
  QMutex* m_pMutex;
  T* m_pTypeWrapped;
};

#endif // LOCKEDTYPE_H
