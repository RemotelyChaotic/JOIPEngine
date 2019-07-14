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
  m_spSystem.reset();
  m_spThread->quit();
  m_spThread->wait();
}
