#include "ThreadUtils.h"

#include <QObject>

//----------------------------------------------------------------------------------------
//
class CPrivateThreadObject : public QObject
{
  Q_OBJECT

public:
  explicit CPrivateThreadObject(QThread* pThread, std::function<void()> fn) :
    QObject(), m_fnFunction(fn)
  {
    moveToThread(pThread);
  }
  ~CPrivateThreadObject() override {}

public slots:
  void Call()
  {
    if (m_fnFunction)
    {
      m_fnFunction();
    }
    deleteLater();
  }

signals:
  void Done();

private:
  std::function<void()> m_fnFunction;
};

#include "ThreadUtils.moc"

//----------------------------------------------------------------------------------------
//
void utils::RunInThread(QThread* pThread, const std::function<void()>& fn)
{
  if (nullptr != pThread)
  {
    CPrivateThreadObject* pObj = new CPrivateThreadObject(pThread, fn);
    const bool bOk = QMetaObject::invokeMethod(pObj, "Call", Qt::QueuedConnection);
    assert(bOk); Q_UNUSED(bOk)
  }
}

//----------------------------------------------------------------------------------------
//
void utils::RunInThreadBlocking(QThread* pThread, const std::function<void()>& fn)
{
  if (nullptr != pThread)
  {
    CPrivateThreadObject* pObj = new CPrivateThreadObject(pThread, fn);
    const bool bOk = QMetaObject::invokeMethod(pObj, "Call", Qt::BlockingQueuedConnection);
    assert(bOk); Q_UNUSED(bOk)
  }
}
