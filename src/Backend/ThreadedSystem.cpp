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
  m_spThread(std::make_unique<QThread>())
{
}

CThreadedSystem::~CThreadedSystem()
{
  m_spThread->quit();
  while (!m_spThread->isFinished())
  {
    m_spThread->wait(5);
  }
}
//----------------------------------------------------------------------------------------
//
void CThreadedSystem::Cleanup()
{
  m_spSystem->Deinitialize();
  m_spSystem.reset();
}
