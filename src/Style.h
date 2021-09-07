#ifndef STYLE_H
#define STYLE_H

#include <QApplication>

namespace joip_style {
  inline const char* c_sDefaultStyleFolder = ":/resources/style/";
  inline const char* c_sStyleFolder = "styles";
  inline const char* c_sQmlStyleSubFolder = "qml";
  inline const char* c_sDefaultStyle = "Default";

  QStringList AvailableStyles();
  void SetStyle(QApplication* pApp);
}

#endif // STYLE_H
