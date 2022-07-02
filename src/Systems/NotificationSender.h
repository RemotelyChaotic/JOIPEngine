#ifndef CNOTIFICATIONSENDER_H
#define CNOTIFICATIONSENDER_H

#include <QObject>
#include <QPointer>

#define Notifier() CNotificationSender::Instance()

class CMainWindow;
#if defined(Q_OS_WINDOWS) && !defined(Q_OS_ANDROID)
class CWindowsNativePushNotification;
#elif defined(Q_OS_ANDROID)
class CAndroidNotificationClient;
#endif

class CNotificationSender : public QObject
{
  Q_OBJECT

public:
  CNotificationSender();
  ~CNotificationSender();

  static CNotificationSender* Instance();

  void SendNotification(const QString& sTitle, const QString& sMsg);
  void SetMainWindow(CMainWindow* pMainWindow);

private slots:
  void SlotSendNotification(const QString& sTitle, const QString& sMsg);

private:
  static CNotificationSender* m_pInstance;

#if defined(Q_OS_WINDOWS) && !defined(Q_OS_ANDROID)
  std::unique_ptr<CWindowsNativePushNotification> m_spNativeNotifications;
#elif defined(Q_OS_ANDROID)
  std::unique_ptr<CAndroidNotificationClient>     m_spNativeNotifications;
#endif

  QPointer<CMainWindow>       m_pMainWindow;
};

#endif // CNOTIFICATIONSENDER_H
