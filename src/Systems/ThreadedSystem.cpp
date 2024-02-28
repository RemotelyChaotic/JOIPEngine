#include "ThreadedSystem.h"

CSystemBase::CSystemBase(QThread::Priority prio) :
  QObject(nullptr),
  m_bInitialized(false),
  m_priority(prio)
{}

CSystemBase::~CSystemBase()
{}

//----------------------------------------------------------------------------------------
//
CThreadedSystem::CThreadedSystem(const QString& sName) :
  QObject(nullptr),
  m_pThread(new QThread(this))
{
  m_pThread->setObjectName(sName);
}

CThreadedSystem::~CThreadedSystem()
{
  if (nullptr != m_pThread)
  {
    m_pThread->quit();
    while (!m_pThread->isFinished())
    {
      m_pThread->wait(5);
    }
  }
}
//----------------------------------------------------------------------------------------
//
void CThreadedSystem::Cleanup()
{
  if (nullptr != m_spSystem)
  {
    m_spSystem->Deinitialize();
  }
}
