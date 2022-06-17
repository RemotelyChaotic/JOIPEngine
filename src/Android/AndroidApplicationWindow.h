#ifndef ANDROIDAPPLICATIONWINDOW_H
#define ANDROIDAPPLICATIONWINDOW_H

#include <QtAndroid>

// WindowManager.LayoutParams
#define FLAG_TRANSLUCENT_NAVIGATION 0x08000000
#define FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS 0x80000000
// View
#define SYSTEM_UI_FLAG_LIGHT_STATUS_BAR 0x00002000

QAndroidJniObject GetAndroidWindow();
void SetAndroidWindowFlags(QAndroidJniObject& window, qint32 iFlags);
void ClearAndroidWindowFlags(QAndroidJniObject& window, qint32 iFlags);

#endif // ANDROIDAPPLICATIONWINDOW_H
