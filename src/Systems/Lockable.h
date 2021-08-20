#ifndef LOCKABLE_H
#define LOCKABLE_H

#include <QReadWriteLock>

class CLockable
{
public:
  explicit CLockable(const CLockable& other) :
    m_pRwLock(other.m_pRwLock)
  {}
  explicit CLockable(QReadWriteLock* pRwLock) :
    m_pRwLock(pRwLock)
  {}

  QReadLocker ReadLocker() const
  {
    if (nullptr != m_pRwLock) { return QReadLocker(m_pRwLock); }
    return QReadLocker(nullptr);
  }

  QWriteLocker WriteLocker() const
  {
    if (nullptr != m_pRwLock) { return QWriteLocker(m_pRwLock); }
    return QWriteLocker(nullptr);
  }

  void LockForRead() const
  {
    if (nullptr != m_pRwLock) { m_pRwLock->lockForRead(); }
  }

  void LockForWrite() const
  {
    if (nullptr != m_pRwLock) { m_pRwLock->lockForWrite(); }
  }

  void Unlock() const
  {
    if (nullptr != m_pRwLock) { m_pRwLock->unlock(); }
  }

  bool TryLockForRead() const
  {
    if (nullptr != m_pRwLock) { return m_pRwLock->tryLockForRead(); }
    return false;
  }

  bool TryLockForRead(int timeout) const
  {
    if (nullptr != m_pRwLock) { return m_pRwLock->tryLockForRead(timeout); }
    return false;
  }

  bool TryLockForWrite() const
  {
    if (nullptr != m_pRwLock) { return m_pRwLock->tryLockForWrite(); }
    return false;
  }

  bool TryLockForWrite(int timeout) const
  {
    if (nullptr != m_pRwLock) { return m_pRwLock->tryLockForWrite(timeout); }
    return false;
  }

  bool IsLockedForRead() const
  {
    if (nullptr != m_pRwLock) { if (TryLockForRead(1)) { Unlock(); return true; } }
    return false;
  }

  bool IsLockedForWrite() const
  {
    if (nullptr != m_pRwLock) { if (TryLockForWrite(1)) { Unlock(); return true; } }
    return false;
  }

protected:
  QReadWriteLock* m_pRwLock;
};

#endif // LOCKABLE_H
