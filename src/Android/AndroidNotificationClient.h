#ifndef CANDROIDNOTIFICATIONCLIENT_H
#define CANDROIDNOTIFICATIONCLIENT_H

#include <QString>

class CAndroidNotificationClient
{
public:
  CAndroidNotificationClient();
  ~CAndroidNotificationClient();

  void ShowNotification(const QString& sTitle, const QString& sMsg);
};

#endif // CANDROIDNOTIFICATIONCLIENT_H
