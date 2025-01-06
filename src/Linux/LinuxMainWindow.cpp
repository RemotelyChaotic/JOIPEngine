#include "LinuxMainWindow.h"
#include "Application.h"
#include "Settings.h"
#include "SVersion.h"
#include "WindowContext.h"

#include <QDir>
#include <QGuiApplication>
#include <QMessageBox>
#include <QScreen>
#include <QStyle>

CLinuxMainWindow::CLinuxMainWindow(QWidget* pParent) :
  CMainWindow(pParent)
{
}

CLinuxMainWindow::~CLinuxMainWindow()
{
}

//----------------------------------------------------------------------------------------
//
void CLinuxMainWindow::OnShow()
{
  // Nothing to do for now.
}

//----------------------------------------------------------------------------------------
//
void CLinuxMainWindow::SlotResolutionChanged()
{
  QPoint globalCursorPos = QCursor::pos();
  QRect availableGeometry =
      QGuiApplication::screenAt(globalCursorPos)->geometry();

  QSize newResolution = m_spSettings->Resolution();
  setGeometry(
      QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter,
          newResolution, availableGeometry));
}

//----------------------------------------------------------------------------------------
//
void CLinuxMainWindow::SlotWindowModeChanged()
{
  CSettings::WindowMode winMode = m_spSettings->GetWindowMode();

  if (CSettings::WindowMode::eBorderless == winMode ||
      CSettings::WindowMode::eFullscreen == winMode)
  {
    setWindowFlag(Qt::FramelessWindowHint, true);
  }
  else
  {
    setWindowFlag(Qt::FramelessWindowHint, false);
  }

  bool bWasFullScreen = windowState() & Qt::WindowFullScreen;

  if (CSettings::WindowMode::eFullscreen == winMode)
  {
    setWindowState(windowState() | Qt::WindowFullScreen);
  }
  else
  {
    setWindowState(windowState() & ~Qt::WindowFullScreen);
  }

  if (bWasFullScreen && CSettings::WindowMode::eFullscreen != winMode)
  {
    SlotResolutionChanged();
  }

  window()->show();
}

//----------------------------------------------------------------------------------------
//
void CLinuxMainWindow::ConnectSlotsImpl()
{
  connect(m_spSettings.get(), &CSettings::fullscreenChanged,
          this, &CLinuxMainWindow::SlotWindowModeChanged, Qt::QueuedConnection);
  connect(m_spSettings.get(), &CSettings::windowModeChanged,
          this, &CLinuxMainWindow::SlotWindowModeChanged, Qt::QueuedConnection);

  connect(m_spSettings.get(), &CSettings::resolutionChanged,
          this, &CLinuxMainWindow::SlotResolutionChanged, Qt::QueuedConnection);
}

//----------------------------------------------------------------------------------------
//
void CLinuxMainWindow::OldSettingsDetected()
{
  SVersion version(VERSION_XYZ);
  QString sContentPath = QCoreApplication::instance()->applicationDirPath() +
      QDir::separator() + ".." + QDir::separator() + "data";
  QFileInfo contentFileInfo(sContentPath);

  if (m_spSettings->ContentFolder() != contentFileInfo.absoluteFilePath())
  {
    QPointer pThis(this);
    QMessageBox msgBox;
    msgBox.setWindowFlags(Qt::FramelessWindowHint);
    msgBox.setText(tr("Settings for old version found."));
    msgBox.setInformativeText(tr("Would you like to update the settings to:<br>"
                                 "<ul>"
                                 "<li>Set the content folder to this applications data folder<br>"
                                 "(You will have to move over any JOI-Projects from the old folder to access them)?</li>"
                                 "</ul>"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    msgBox.setModal(true);
    qint32 iRetVal = msgBox.exec();
    if (pThis.isNull()) { return; }
    if (QMessageBox::Yes == iRetVal)
    {
      m_spSettings->SetContentFolder(contentFileInfo.absoluteFilePath());
    }
    m_spSettings->SetSettingsVersion(QT_VERSION_CHECK(version.m_iMajor, version.m_iMinor, version.m_iPatch));
  }
  else
  {
    m_spSettings->SetSettingsVersion(QT_VERSION_CHECK(version.m_iMajor, version.m_iMinor, version.m_iPatch));
  }
}
