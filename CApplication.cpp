#include "CApplication.h"

CApplication::CApplication(int argc, char *argv[]) :
  QGuiApplication(argc, argv),
  m_spSettings(nullptr),
  m_bInitialized(false)
{

}

CApplication::~CApplication()
{

}

//----------------------------------------------------------------------------------------
//
void CApplication::Initialize()
{
  m_spSettings = std::make_shared<CSettings>();
  m_bInitialized = true;
}
