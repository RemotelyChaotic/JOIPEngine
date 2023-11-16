#ifndef THEMES_H
#define THEMES_H

#include <QApplication>

namespace joip_style
{
  inline const char* c_sCodeThemFolder = "themes";

  QStringList AvailableThemes();
  QString ThemeFolder();
}

#endif // THEMES_H
