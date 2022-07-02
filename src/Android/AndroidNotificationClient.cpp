#include "AndroidNotificationClient.h"

#include <QtAndroid>

CAndroidNotificationClient::CAndroidNotificationClient()
{

}
CAndroidNotificationClient::~CAndroidNotificationClient() = default;

//----------------------------------------------------------------------------------------
//
void CAndroidNotificationClient::ShowNotification(const QString& sTitle, const QString& sMsg)
{
  QAndroidJniObject javaTitle = QAndroidJniObject::fromString(sTitle);
  QAndroidJniObject javaNotification = QAndroidJniObject::fromString(sMsg);
  QAndroidJniObject::callStaticMethod<void>(
          "org.joipengine/JOIPNotificationClient",
          "notify",
          "(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;)V",
          QtAndroid::androidContext().object(),
          javaTitle.object<jstring>(),
          javaNotification.object<jstring>());
}
