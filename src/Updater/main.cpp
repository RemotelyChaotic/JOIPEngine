#include "Application.h"
#include "SettingsData.h"
#include "Splash.h"
#include "Style.h"

#include <QApplication>
#include <QCommandLineParser>

namespace
{
  const QString c_sOrganisation = "Private";
  const QString c_sApplicationName = "JOIPEngine";

  const QString c_sVersion = "General/version";

  const QString c_sSettingStyle = "Graphics/style";
  const QString c_sSettingFont = "Graphics/font";
}

//----------------------------------------------------------------------------------------
//
SSettingsData CheckSettings()
{
  SSettingsData ret;

  bool bNeedsSynch = false;
  ret.spSettings = std::make_shared<QSettings>(QSettings::IniFormat, QSettings::UserScope,
                                                c_sOrganisation, c_sApplicationName);

  // get version of settings
  qint32 iCurrentSettingsVersion;
  if (!ret.spSettings->contains(c_sVersion))
  {
    iCurrentSettingsVersion = 0;
  }
  else
  {
    iCurrentSettingsVersion = ret.spSettings->value(c_sVersion).toUInt();
  }

  // check font
  if (!ret.spSettings->contains(c_sSettingFont))
  {
    bNeedsSynch = true;
    ret.spSettings->setValue(c_sSettingFont, "Arial");
  }

  // check style
  if (!ret.spSettings->contains(c_sSettingStyle))
  {
    bNeedsSynch = true;
    ret.spSettings->setValue(c_sSettingStyle, "Blue Night");
  }

  // check auto-update settings
  if (!ret.spSettings->contains(c_sSettingAutoUpdate))
  {
    bNeedsSynch = true;
    ret.spSettings->setValue(c_sSettingAutoUpdate, true);
    ret.bAutoUpdateFound = false;
  }
  else
  {
    ret.bAutoUpdateFound = true;
  }

  // write file if nesseccary
  if (bNeedsSynch)
  {
    ret.spSettings->sync();
  }

  ret.version = SVersion(iCurrentSettingsVersion);
  ret.sFont = ret.spSettings->value(c_sSettingFont).toString();
  ret.sStyle = ret.spSettings->value(c_sSettingStyle).toString();
  ret.bAutoUpdate = ret.spSettings->value(c_sSettingAutoUpdate).toBool();

  return ret;
}

//----------------------------------------------------------------------------------------
//
int main(int argc, char *argv[])
{
  QStringList args;
  for (qint32 i = 1; argc > i; ++i)
  {
    args << QString(argv[i]);
  }

  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);

  CApplication app(argc, argv);

  QCommandLineParser parser;
  QCommandLineOption continueOption(QStringList() << "c" << "continue",
                                    QCoreApplication::translate("main", "Overwrite existing files."));
  parser.addOption(continueOption);
  QCommandLineOption targetVersionOption(QStringList() << "t" << "target-version",
                                         QCoreApplication::translate("main", "The new version."),
                                         QCoreApplication::translate("main", "version"));
  parser.addOption(targetVersionOption);
  parser.process(app);

  SSettingsData settingsData = CheckSettings();
  settingsData.bContinueUpdate = parser.isSet(continueOption);
  settingsData.targetVersion = parser.value(targetVersionOption);

  joip_style::SetStyle(&app, settingsData.sStyle, settingsData.sFont);
  app.SetSettings(&settingsData);

  std::unique_ptr<CSplash> spW = std::make_unique<CSplash>(settingsData);
  spW->show();

  qint32 iRetVal = app.exec();
  return iRetVal;
}
