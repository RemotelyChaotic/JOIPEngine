#include "Application.h"
#include <cassert>

CApplication::CApplication(int argc, char *argv[]) :
  QApplication(argc, argv),
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
  m_bInitialized = true;
}
