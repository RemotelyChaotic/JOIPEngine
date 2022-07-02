#include "AndroidNavigationBar.h"
#include "AndroidApplicationWindow.h"

QColor CAndroidNavigationBar::m_color = QColor(Qt::black);

//----------------------------------------------------------------------------------------
//
CAndroidNavigationBar::CAndroidNavigationBar(QObject* pParent) :
  QObject(pParent)
{

}

//----------------------------------------------------------------------------------------
//
bool CAndroidNavigationBar::IsAvailable()
{
  return QtAndroid::androidSdkVersion() >= 21;
}

//----------------------------------------------------------------------------------------
//
void CAndroidNavigationBar::SetBarVisiblility(bool bVisible)
{
  if (!IsAvailable()) { return; }

  QtAndroid::runOnAndroidThread([=]() {
      QAndroidJniObject activity = GetAndroidActivity();
      activity.callMethod<void>("setNavigationVisible", "(Z)V", bVisible);
  });
}

//----------------------------------------------------------------------------------------
//
QColor CAndroidNavigationBar::Color()
{
  return m_color;
}

//----------------------------------------------------------------------------------------
//
void CAndroidNavigationBar::SetColor(const QColor &color)
{
  if (!IsAvailable()) { return; }

  m_color = color;
  QtAndroid::runOnAndroidThread([=]() {
      QAndroidJniObject window = GetAndroidWindow();
      window.callMethod<void>("setNavigationBarColor", "(I)V", color.rgba());
  });
}
