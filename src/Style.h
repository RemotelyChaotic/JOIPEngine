#ifndef STYLE_H
#define STYLE_H

#include <QApplication>

namespace joip_style {
#if defined(Q_OS_ANDROID)
  inline const char* c_sDefaultAndroidStyleFolder = ":/android/resources/android_styles";
#endif
  inline const char* c_sDefaultStyleFolder = ":/resources/style/";
  inline const char* c_sStyleFolder = "styles";
  inline const char* c_sQmlStyleSubFolder = "qml";
  inline const char* c_sDefaultStyle = "Default";

  QStringList AvailableStyles();
  QString StyleFile(const QString& sStyle);
  QString StyleFolder();
  void SetStyle(QApplication* pApp, const QString& sStyle, const QString& sFont);
}

#endif // STYLE_H
