#ifndef SETTINGSDATA_H
#define SETTINGSDATA_H

#include "SVersion.h"

#include <QSettings>
#include <memory>

struct SSettingsData
{
  std::shared_ptr<QSettings> spSettings = nullptr;

  SVersion version;
  QString sStyle;
  QString sFont;
  bool    bAutoUpdateFound;
  bool    bAutoUpdate;

  bool    bContinueUpdate = false;
  SVersion targetVersion;

  QString Font() const { return sFont; }
};

#endif // SETTINGSDATA_H
