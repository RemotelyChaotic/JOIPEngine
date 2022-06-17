#include "MainWindowFactory.h"

#if defined(Q_OS_ANDROID)
#include "Android/AndroidMainWindow.h"
#elif defined(WIN32)
#include "Windows/WindowsMainWindow.h"

#include <QtPlatformHeaders/QWindowsWindowFunctions>
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
#elif defined(WIN32)
  auto spMw = std::make_unique<CWindowsMainWindow>(pParent);

  // Fixes problems with OpenGL based windows
  // https://doc.qt.io/qt-5/windows-issues.html#fullscreen-opengl-based-windows
  QWindowsWindowFunctions::setHasBorderInFullScreen(spMw->windowHandle(), true);

  return spMw;
#else
  return nullptr;
#endif
}

//----------------------------------------------------------------------------------------
//
CMainWindowFactory::CMainWindowFactory()
{
}
