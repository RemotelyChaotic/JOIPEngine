#include "Application.h"

CApplication::CApplication(int &argc, char **argv) :
    QApplication(argc, argv)
{
}

CApplication::~CApplication() {}

//----------------------------------------------------------------------------------------
//
CApplication* CApplication::Instance()
{
  return dynamic_cast<CApplication*>(qApp);
}

//----------------------------------------------------------------------------------------
//
void CApplication::SetSettings(SSettingsData* pSettings)
{
  m_pSettings = pSettings;
}

//----------------------------------------------------------------------------------------
//
SSettingsData* CApplication::Settings() const
{
  return m_pSettings;
}
