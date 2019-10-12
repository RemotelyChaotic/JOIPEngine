#include "ThreadedSystem.h"

CThreadedObject::CThreadedObject() :
  QObject(nullptr),
  m_bInitialized(false)
{}

CThreadedObject::~CThreadedObject()
{}

//----------------------------------------------------------------------------------------
//
CThreadedSystem::CThreadedSystem() :
  QObject(nullptr),
  m_pThread(new QThread(this))
{
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
