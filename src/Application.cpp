#include "Application.h"
#include "Enums.h"
#include "Backend/ThreadedSystem.h"
#include <cassert>

CApplication::CApplication(int argc, char *argv[]) :
  QApplication(argc, argv),
  m_spSystemsMap(),
  m_spSettings(nullptr),
  m_bInitialized(false)
{

}

CApplication::~CApplication()
{

}

//----------------------------------------------------------------------------------------
//
CApplication* CApplication::Instance()
{
  CApplication* pApp = dynamic_cast<CApplication*>(QCoreApplication::instance());
  assert(nullptr != pApp);
  return pApp;
}

//----------------------------------------------------------------------------------------
//
void CApplication::Initialize()
{
  m_spSettings = std::make_shared<CSettings>();

  // create subsystems
  m_spSystemsMap.insert({ECoreSystems::eDatabaseManager, std::make_shared<CThreadedSystem>()});

  // init subsystems
  m_spSystemsMap[ECoreSystems::eDatabaseManager]->RegisterObject<CDatabaseManager>();

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
template<>
std::weak_ptr<CDatabaseManager> CApplication::System<CDatabaseManager>()
{
  return std::static_pointer_cast<CDatabaseManager>(m_spSystemsMap[ECoreSystems::eDatabaseManager]->Get());
}

