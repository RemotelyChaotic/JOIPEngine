#include "NotificationSender.h"
#include "Application.h"
#include "MainWindow.h"

#include "Widgets/PushNotification.h"
#if defined(Q_OS_WINDOWS) && !defined(Q_OS_ANDROID)
#include "Windows/WindowsNativePushNotification.h"
#elif defined(Q_OS_ANDROID)
#include "Android/AndroidNotificationClient.h"
#endif

CNotificationSender* CNotificationSender::m_pInstance = nullptr;

//----------------------------------------------------------------------------------------
//
CNotificationSender::CNotificationSender() :
  QObject(nullptr),
#if defined(Q_OS_WINDOWS) && !defined(Q_OS_ANDROID)
  m_spNativeNotifications(std::make_unique<CWindowsNativePushNotification>(nullptr)),
#elif defined(Q_OS_ANDROID)
  m_spNativeNotifications(std::make_unique<CAndroidNotificationClient>()),
#endif
  m_spSettings(nullptr),
  m_pMainWindow(nullptr)
{
  m_pInstance = this;
}
CNotificationSender::~CNotificationSender()
{
  m_pInstance = nullptr;
}

//----------------------------------------------------------------------------------------
//
CNotificationSender* CNotificationSender::Instance()
{
  return m_pInstance;
}

//----------------------------------------------------------------------------------------
//
void CNotificationSender::SendNotification(const QString& sTitle,
                                           const QString& sMsg,
                                           bool bExternalOnly)
{
  bool bOk =
      QMetaObject::invokeMethod(this, "SlotSendNotification", Qt::QueuedConnection,
                                       Q_ARG(QString, sTitle), Q_ARG(QString, sMsg),
                                       Q_ARG(bool, bExternalOnly));
  assert(bOk);
  Q_UNUSED(bOk)
}

//----------------------------------------------------------------------------------------
//
void CNotificationSender::SetMainWindow(CMainWindow* pMainWindow)
{
  m_pMainWindow = pMainWindow;
#if defined(Q_OS_WINDOWS)
  m_spNativeNotifications->SetMainWindow(pMainWindow);
#endif
  m_spSettings = CApplication::Instance()->Settings();
}

//----------------------------------------------------------------------------------------
//
void CNotificationSender::SlotSendNotification(const QString& sTitle, const QString& sMsg,
                                               bool bExternalOnly)
{
  using namespace std::chrono_literals;
  [[maybe_unused]] const auto showTime = 5s;

  if (!m_spSettings->PushNotifications())
  {
    SoftwarePushNotifications(sTitle, sMsg);
  }
  else
  {
#if (defined(Q_OS_WIN) || defined(Q_OS_LINUX)) && !defined(Q_OS_ANDROID)
    if (nullptr != m_pMainWindow &&
        Qt::ApplicationActive == CApplication::Instance()->applicationState() &&
        !bExternalOnly)
    {
      SoftwarePushNotifications(sTitle, sMsg);
    }
    else
    {
  #if defined(Q_OS_WIN)
      m_spNativeNotifications->Show(sTitle, sMsg, showTime);
  #endif
    }
#elif defined(Q_OS_ANDROID)
    m_spNativeNotifications->ShowNotification(sTitle, sMsg);
#else
    SoftwarePushNotifications(sTitle, sMsg);
#endif
  }
}

//----------------------------------------------------------------------------------------
//
void CNotificationSender::SoftwarePushNotifications(const QString& /*sTitle*/, const QString& sMsg)
{
  if (nullptr != m_pMainWindow)
  {
    using namespace std::chrono_literals;
    const auto showTime = 5s;

    CPushNotification* pNotification =
        new CPushNotification(sMsg, showTime, true, m_pMainWindow.data());
    pNotification->Climb();
    pNotification->Move(pNotification->x(), pNotification->height());
    pNotification->Resize();
    pNotification->Show();
  }
}
