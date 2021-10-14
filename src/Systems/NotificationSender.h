#ifndef CNOTIFICATIONSENDER_H
#define CNOTIFICATIONSENDER_H

#include <QObject>
#include <QPointer>

#define Notifier() CNotificationSender::Instance()

class CMainWindow;

class CNotificationSender : public QObject
{
  Q_OBJECT

public:
  CNotificationSender();
  ~CNotificationSender();

  static CNotificationSender* Instance();

  void SendNotification(const QString& sMsg);
  void SetMainWindow(CMainWindow* pMainWindow);

private slots:
  void SlotSendNotification(const QString& sMsg);

private:
  static CNotificationSender* m_pInstance;
  QPointer<CMainWindow>       m_pMainWindow;
};

#endif // CNOTIFICATIONSENDER_H
