#include "MainWindowFactory.h"

#if defined(Q_OS_ANDROID)
#include "Android/AndroidMainWindow.h"
#elif defined(WIN32)
#include "Windows/WindowsMainWindow.h"
#elif defined(Q_OS_LINUX)
#include "Linux/LinuxMainWindow.h"
#else
#endif

//----------------------------------------------------------------------------------------
//
CMainWindowFactory& CMainWindowFactory::Instance()
{
  static CMainWindowFactory instance;
  return instance;
}

//----------------------------------------------------------------------------------------
//
std::unique_ptr<CMainWindowBase> CMainWindowFactory::CreateMainWindow(QWidget* pParent)
{
#if defined(Q_OS_ANDROID)
  return std::make_unique<CAndroidMainWindow>(pParent);
#elif defined(Q_OS_WIN) && !defined(Q_OS_ANDROID)
  return std::make_unique<CWindowsMainWindow>(pParent);
#elif defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
  return std::make_unique<CLinuxMainWindow>(pParent);
#else
  return nullptr;
#endif
}

//----------------------------------------------------------------------------------------
//
CMainWindowFactory::CMainWindowFactory()
{
}
