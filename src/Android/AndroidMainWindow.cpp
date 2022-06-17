#include "AndroidMainWindow.h"
#include "AndroidNavigationBar.h"
#include "Settings.h"
#include "SVersion.h"

#include <QGuiApplication>
#include <QScreen>
#include <QStyle>

CAndroidMainWindow::CAndroidMainWindow(QWidget* pParent) :
  CMainWindow(pParent),
  m_pNavigationBar(new CAndroidNavigationBar(this))
{
}

CAndroidMainWindow::~CAndroidMainWindow()
{
}

//----------------------------------------------------------------------------------------
//
QColor CAndroidMainWindow::NavigationColor() const
{
  return m_pNavigationBar->Color();
}

//----------------------------------------------------------------------------------------
//
void CAndroidMainWindow::SetNavigationColor(const QColor& color)
{
  m_pNavigationBar->SetColor(color);
}

//----------------------------------------------------------------------------------------
//
void CAndroidMainWindow::SlotResolutionChanged()
{
  QRect availableGeometry =
      QGuiApplication::screenAt({0,0})->geometry();
  setFixedSize(availableGeometry.size());
}

//----------------------------------------------------------------------------------------
//
void CAndroidMainWindow::SlotWindowModeChanged()
{
  setWindowFlag(Qt::FramelessWindowHint);
  setWindowState(windowState() | Qt::WindowFullScreen);
  showMaximized();
}

//----------------------------------------------------------------------------------------
//
void CAndroidMainWindow::ConnectSlotsImpl()
{
}

//----------------------------------------------------------------------------------------
//
void CAndroidMainWindow::OldSettingsDetected()
{
  SVersion version(VERSION_XYZ);
  m_spSettings->SetSettingsVersion(QT_VERSION_CHECK(version.m_iMajor, version.m_iMinor, version.m_iPatch));
}
