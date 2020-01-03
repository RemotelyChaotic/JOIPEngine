#ifndef STYLE_H
#define STYLE_H

#include <QApplication>

namespace joip_style {
  const QString c_sDefaultStyle = "Default";

  QStringList AvailableStyles();
  void SetStyle(QApplication* pApp);
}

#endif // STYLE_H
