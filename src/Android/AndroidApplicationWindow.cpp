#include "AndroidApplicationWindow.h"

//----------------------------------------------------------------------------------------
//
QAndroidJniObject GetAndroidWindow()
{
  QAndroidJniObject window = QtAndroid::androidActivity().callObjectMethod("getWindow", "()Landroid/view/Window;");
  return window;
}

//----------------------------------------------------------------------------------------
//
QAndroidJniObject GetAndroidActivity()
{
  return QtAndroid::androidActivity();
}

//----------------------------------------------------------------------------------------
//
void SetAndroidWindowFlags(QAndroidJniObject& window, qint32 iFlags)
{
  window.callMethod<void>("addFlags", "(I)V", iFlags);
}

//----------------------------------------------------------------------------------------
//
void ClearAndroidWindowFlags(QAndroidJniObject& window, qint32 iFlags)
{
  window.callMethod<void>("clearFlags", "(I)V", iFlags);
}

//----------------------------------------------------------------------------------------
//
bool IsAppRunning(QAndroidJniObject& activity, QString sApplication)
{
  QAndroidJniObject string = QAndroidJniObject::fromString(sApplication);
  return activity.callMethod<jboolean>("isAppRunning", "(Ljava/lang/String;)I", string.object<jstring>());
}
