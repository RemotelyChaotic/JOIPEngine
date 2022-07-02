#include "WindowsNativePushNotification.h"
#include "Application.h"
#include "MainWindow.h"
#include "Settings.h"
#include "SVersion.h"

#include <QDebug>
#include <QFileInfo>
#include <QObject>

using namespace WinToastLib;

class CWinToastHandler : public WinToastLib::IWinToastHandler
{
public:
  CWinToastHandler() {}
  ~CWinToastHandler() = default;

  void toastActivated() const override {}
  void toastActivated(int actionIndex) const override {}
  void toastDismissed(WinToastDismissalReason state) const override {}
  void toastFailed() const override {}
};

//----------------------------------------------------------------------------------------
//
CWindowsNativePushNotification::CWindowsNativePushNotification(QPointer<CMainWindow> pMainWindow) :
  m_spTempl(std::make_unique<WinToastTemplate>(WinToastTemplate::ImageAndText02)),
  m_spToastHandler(std::make_shared<CWinToastHandler>()),
  m_pMainWindow(pMainWindow)
{
  if (!WinToast::isCompatible())
  {
    qWarning() << QObject::tr("Windows does not support toast notifications.");
  }
  else
  {
    WinToast::instance()->setAppName(CSettings::c_sApplicationName.toStdWString());
    const auto aumi = WinToast::configureAUMI(CSettings::c_sOrganisation.toStdWString(),
                                              CSettings::c_sApplicationName.toStdWString(),
                                              L"toast",
                                              static_cast<QString>(SVersion(VERSION)).toStdWString());
    WinToast::instance()->setAppUserModelId(aumi);

    if (!WinToast::instance()->initialize())
    {
      qWarning() << QObject::tr("Could not initialize toast.");
    }
  }
}

CWindowsNativePushNotification::~CWindowsNativePushNotification() = default;

//----------------------------------------------------------------------------------------
//
void CWindowsNativePushNotification::SetMainWindow(QPointer<CMainWindow> pMainWindow)
{
  m_pMainWindow = pMainWindow;
}

//----------------------------------------------------------------------------------------
//
void CWindowsNativePushNotification::Show(const QString& sTitle, const QString& sMsg,
                                          std::chrono::milliseconds displayTime)
{
  if (WinToast::isCompatible() && WinToast::instance()->isInitialized())
  {
    m_spTempl->setTextField(sTitle.toStdWString(), WinToastTemplate::FirstLine);
    m_spTempl->setTextField(sMsg.toStdWString(), WinToastTemplate::SecondLine);
    m_spTempl->setImagePath(QFileInfo("Icon.ico").absoluteFilePath().toStdWString());
    if (!WinToast::instance()->showToast(*m_spTempl, m_spToastHandler))
    {
      qWarning() << QObject::tr("Could not launch your toast notification!");
    }
  }

  if (nullptr != CApplication::Instance() &&
      nullptr != m_pMainWindow)
  {
    CApplication::Instance()->alert(m_pMainWindow.data(), displayTime.count());
  }
}
