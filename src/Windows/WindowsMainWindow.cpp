#include "WindowsMainWindow.h"
#include "Application.h"
#include "Settings.h"
#include "SVersion.h"
#include "WindowContext.h"

#include "Systems/ProjectDownloader.h"

#include <QDir>
#include <QGuiApplication>
#include <QMessageBox>
#include <QScreen>
#include <QStyle>
#include <QWinTaskbarButton>
#include <QWinTaskbarProgress>
#include <QtPlatformHeaders/QWindowsWindowFunctions>

CWindowsMainWindow::CWindowsMainWindow(QWidget* pParent) :
  CMainWindow(pParent)
{
  m_pTaskButton = new QWinTaskbarButton(this);
}

CWindowsMainWindow::~CWindowsMainWindow()
{

}

//----------------------------------------------------------------------------------------
//
void CWindowsMainWindow::OnShow()
{
  m_pTaskButton->setWindow(windowHandle());

  m_pTaskProgress = m_pTaskButton->progress();
  m_pTaskProgress->setVisible(false);

  // Fixes problems with OpenGL based windows
  // https://doc.qt.io/qt-5/windows-issues.html#fullscreen-opengl-based-windows
  QWindowsWindowFunctions::setHasBorderInFullScreen(windowHandle(), true);
}

//----------------------------------------------------------------------------------------
//
void CWindowsMainWindow::SlotResolutionChanged()
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
void CWindowsMainWindow::SlotWindowModeChanged()
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
void CWindowsMainWindow::ConnectSlotsImpl()
{
  connect(m_spSettings.get(), &CSettings::fullscreenChanged,
          this, &CWindowsMainWindow::SlotWindowModeChanged, Qt::QueuedConnection);
  connect(m_spSettings.get(), &CSettings::windowModeChanged,
          this, &CWindowsMainWindow::SlotWindowModeChanged, Qt::QueuedConnection);

  connect(m_spSettings.get(), &CSettings::resolutionChanged,
          this, &CWindowsMainWindow::SlotResolutionChanged, Qt::QueuedConnection);

  connect(m_spWindowContext.get(), &CWindowContext::SignalChangeAppOverlay,
          this, &CWindowsMainWindow::SlotChangeAppOverlay);

  if (auto spDownloader = CApplication::Instance()->System<CProjectDownloader>().lock())
  {
    connect(spDownloader.get(), &CProjectDownloader::SignalDownloadFinished,
            this, &CWindowsMainWindow::SlotDownloadFinished, Qt::QueuedConnection);
    connect(spDownloader.get(), &CProjectDownloader::SignalDownloadStarted,
            this, &CWindowsMainWindow::SlotDownloadStarted, Qt::QueuedConnection);
    connect(spDownloader.get(), &CProjectDownloader::SignalProgressChanged,
            this, &CWindowsMainWindow::SlotProgressChanged, Qt::QueuedConnection);
  }
}

//----------------------------------------------------------------------------------------
//
void CWindowsMainWindow::OldSettingsDetected()
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

//----------------------------------------------------------------------------------------
//
void CWindowsMainWindow::SlotChangeAppOverlay(const QString& sImage)
{
  if (nullptr == m_pTaskButton) { return; }

  QFileInfo info(sImage);
  if (sImage.isEmpty() || !info.exists())
  {
    m_pTaskButton->setOverlayIcon(QIcon());
  }
  else
  {
    m_pTaskButton->setOverlayIcon(QIcon(sImage));
  }
}

//----------------------------------------------------------------------------------------
//
void CWindowsMainWindow::SlotDownloadFinished(qint32 iProjId)
{
  if (nullptr == m_pTaskProgress) { return; }

  Q_UNUSED(iProjId)
  m_pTaskProgress->setVisible(false);
}

//----------------------------------------------------------------------------------------
//
void CWindowsMainWindow::SlotDownloadStarted(qint32 iProjId)
{
  if (nullptr == m_pTaskProgress) { return; }

  Q_UNUSED(iProjId)
  m_pTaskProgress->setVisible(true);
  m_pTaskProgress->setValue(0);
}

//----------------------------------------------------------------------------------------
//
void CWindowsMainWindow::SlotProgressChanged(qint32 iProjId, qint32 iProgress)
{
  if (nullptr == m_pTaskProgress) { return; }

  Q_UNUSED(iProjId)
  m_pTaskProgress->setValue(iProgress);
}
