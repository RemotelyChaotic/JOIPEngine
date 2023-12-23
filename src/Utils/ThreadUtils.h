#ifndef CTHREAD_H
#define CTHREAD_H

#include <QThread>

#include <functional>

namespace utils
{
  void RunInThread(QThread* pThread, const std::function<void()>& fn);
  void RunInThreadBlocking(QThread* pThread, const std::function<void()>& fn);
}

#endif // CTHREAD_H
