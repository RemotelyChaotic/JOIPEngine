#include "Settings.h"
#include "Application.h"
#include "Style.h"
#include "SVersion.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QLibraryInfo>
#include <QMutexLocker>
#include <QSettings>
#include <map>

namespace {
  QStringList c_vsKeyBindings = {
    "Pause",
    "Skip",
    "Save",
    "Export",
    "Help",
    "Exit",

    "Answer_1",
    "Answer_2",
    "Answer_3",
    "Answer_4",
    "Answer_5",
    "Answer_6",
    "Answer_7",
    "Answer_8",
    "Answer_9",

    "Left_1",
    "Left_2",
    "Left_3",
    "Left_4",
    "Left_5",
    "Left_6",
    "Left_7",
    "Left_8",
    "Left_9",

    "Right_1",
    "Right_2",
    "Right_3",
    "Right_4",
    "Right_5",
    "Right_6",
    "Right_7",
    "Right_8",
    "Right_9",

    "LeftTab_Resource",
    "LeftTab_MediaPlayer",
    "LeftTab_Settings",
    "LeftTab_Nodes",
    "LeftTab_Code",

    "RightTab_Resource",
    "RightTab_MediaPlayer",
    "RightTab_Settings",
    "RightTab_Nodes",
    "RightTab_Code",

    "Cut",
    "Copy",
    "Pase",
    "Undo",
    "Redo",
    "Search",
    "Backward",
    "Foreward"
  };
}

//----------------------------------------------------------------------------------------
//
const QString CSettings::c_sSettingContentFolder = "Content/folder";
const QString CSettings::c_sSettingFont = "Graphics/font";
const QString CSettings::c_sSettingFullscreen = "Graphics/fullscreen";
const QString CSettings::c_sSettingKeyBindings = "KeyBindings/";
const QString CSettings::c_sSettingMuted = "Audio/muted";
const QString CSettings::c_sSettingOffline = "Content/offline";
const QString CSettings::c_sSettingResolution = "Graphics/resolution";
const QString CSettings::c_sSettingStyle = "Graphics/style";
const QString CSettings::c_sSettingVolume = "Audio/volume";

const QString CSettings::c_sOrganisation = "Private";
const QString CSettings::c_sApplicationName = "JOIPEngine";

//----------------------------------------------------------------------------------------
//
CSettings::CSettings(QObject* pParent) :
  QObject (pParent),
  m_spSettings(std::make_shared<QSettings>(QSettings::IniFormat, QSettings::UserScope,
     CSettings::c_sOrganisation, CSettings::c_sApplicationName)),

  c_sDefaultKeyBindings({
      { "Pause",      QKeySequence(Qt::Key_P)                        },
      { "Skip",       QKeySequence(Qt::Key_Space)                    },
      { "Save",       QKeySequence(QKeySequence::Save)               },
      { "Export",     QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_S) },
      { "Help",       QKeySequence(Qt::Key_H)                        },
      { "Exit",       QKeySequence(Qt::Key_Escape)                   },

      { "Answer_1",   QKeySequence(Qt::Key_1)            },
      { "Answer_2",   QKeySequence(Qt::Key_2)            },
      { "Answer_3",   QKeySequence(Qt::Key_3)            },
      { "Answer_4",   QKeySequence(Qt::Key_4)            },
      { "Answer_5",   QKeySequence(Qt::Key_5)            },
      { "Answer_6",   QKeySequence(Qt::Key_6)            },
      { "Answer_7",   QKeySequence(Qt::Key_7)            },
      { "Answer_8",   QKeySequence(Qt::Key_8)            },
      { "Answer_9",   QKeySequence(Qt::Key_9)            },

      { "Left_1",     QKeySequence(Qt::Key_1)            },
      { "Left_2",     QKeySequence(Qt::Key_2)            },
      { "Left_3",     QKeySequence(Qt::Key_3)            },
      { "Left_4",     QKeySequence(Qt::Key_4)            },
      { "Left_5",     QKeySequence(Qt::Key_5)            },
      { "Left_6",     QKeySequence(Qt::Key_6)            },
      { "Left_7",     QKeySequence(Qt::Key_7)            },
      { "Left_8",     QKeySequence(Qt::Key_8)            },
      { "Left_9",     QKeySequence(Qt::Key_9)            },

      { "Right_1",    QKeySequence(Qt::CTRL + Qt::Key_1) },
      { "Right_2",    QKeySequence(Qt::CTRL + Qt::Key_2) },
      { "Right_3",    QKeySequence(Qt::CTRL + Qt::Key_3) },
      { "Right_4",    QKeySequence(Qt::CTRL + Qt::Key_4) },
      { "Right_5",    QKeySequence(Qt::CTRL + Qt::Key_5) },
      { "Right_6",    QKeySequence(Qt::CTRL + Qt::Key_6) },
      { "Right_7",    QKeySequence(Qt::CTRL + Qt::Key_7) },
      { "Right_8",    QKeySequence(Qt::CTRL + Qt::Key_8) },
      { "Right_9",    QKeySequence(Qt::CTRL + Qt::Key_9) },

      { "LeftTab_Resource",     QKeySequence(Qt::CTRL + Qt::Key_R) },
      { "LeftTab_MediaPlayer",  QKeySequence(Qt::CTRL + Qt::Key_M) },
      { "LeftTab_Settings",     QKeySequence(Qt::CTRL + Qt::Key_T) },
      { "LeftTab_Nodes",        QKeySequence(Qt::CTRL + Qt::Key_N) },
      { "LeftTab_Code",         QKeySequence(Qt::CTRL + Qt::Key_D) },

      { "RightTab_Resource",    QKeySequence(Qt::ALT + Qt::Key_R)  },
      { "RightTab_MediaPlayer", QKeySequence(Qt::ALT + Qt::Key_M)  },
      { "RightTab_Settings",    QKeySequence(Qt::ALT + Qt::Key_T)  },
      { "RightTab_Nodes",       QKeySequence(Qt::ALT + Qt::Key_N)  },
      { "RightTab_Code",        QKeySequence(Qt::ALT + Qt::Key_D)  },

      { "Cut",        QKeySequence(QKeySequence::Cut)    },
      { "Copy",       QKeySequence(QKeySequence::Copy)   },
      { "Pase",       QKeySequence(QKeySequence::Paste)  },
      { "Undo",       QKeySequence(QKeySequence::Undo)   },
      { "Redo",       QKeySequence(QKeySequence::Redo)   },
      { "Search",     QKeySequence(QKeySequence::Find)   },
      { "Backward",   QKeySequence(QKeySequence::Back)   },
      { "Foreward",   QKeySequence(QKeySequence::Forward)}
    })
{
  GenerateSettingsIfNotExists();
}

//----------------------------------------------------------------------------------------
//
CSettings::~CSettings()
{

}

//----------------------------------------------------------------------------------------
//
void CSettings::SetContentFolder(const QString& sPath)
{
  QMutexLocker locker(&m_settingsMutex);
  QString sOldPath = m_spSettings->value(CSettings::c_sSettingContentFolder).toString();
  QFileInfo contentFileInfo(sPath);

  if (!contentFileInfo.exists() || sOldPath == sPath) { return; }

  m_spSettings->setValue(CSettings::c_sSettingContentFolder, sPath);

  emit contentFolderChanged();
}

//----------------------------------------------------------------------------------------
//
QString CSettings::ContentFolder()
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(CSettings::c_sSettingContentFolder).toString();
}

//----------------------------------------------------------------------------------------
//
void CSettings::SetFont(const QString& sFont)
{
  QMutexLocker locker(&m_settingsMutex);
  QString sOldFont = m_spSettings->value(CSettings::c_sSettingFont).toString();

  if (sOldFont == sFont) { return; }

  m_spSettings->setValue(CSettings::c_sSettingFont, sFont);

  emit fontChanged();
}

//----------------------------------------------------------------------------------------
//
QString CSettings::Font()
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(CSettings::c_sSettingFont).toString();
}

//----------------------------------------------------------------------------------------
//
void CSettings::SetFullscreen(bool bValue)
{
  QMutexLocker locker(&m_settingsMutex);

  bool bFullscreen = m_spSettings->value(CSettings::c_sSettingFullscreen).toBool();

  if (bFullscreen == bValue) { return; }

  m_spSettings->setValue(CSettings::c_sSettingFullscreen, bValue);

  emit fullscreenChanged();
}

//----------------------------------------------------------------------------------------
//
bool CSettings::Fullscreen()
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(CSettings::c_sSettingFullscreen).toBool();
}

//----------------------------------------------------------------------------------------
//
QStringList CSettings::KeyBindings()
{
  return c_vsKeyBindings;
}

//----------------------------------------------------------------------------------------
//
QKeySequence CSettings::keyBinding(const QString& sRole)
{
  QMutexLocker locker(&m_settingsMutex);

  // dur wenn in den defaults ein Eintrag existiert, kann dies verwendet werden
  auto it = c_sDefaultKeyBindings.find(sRole);
  if (c_sDefaultKeyBindings.end() != it)
  {
    if (m_spSettings->contains(CSettings::c_sSettingKeyBindings + sRole))
    {
      return QKeySequence(m_spSettings->value(CSettings::c_sSettingKeyBindings + sRole,
                                              it->second.toString()).toString());
    }
    else
    {
      return it->second;
    }
  }
  else
  {
    return QKeySequence();
  }
}

//----------------------------------------------------------------------------------------
//
void CSettings::setKeyBinding(const QKeySequence& sKeySequence, const QString& sRole)
{
  QMutexLocker locker(&m_settingsMutex);

  auto it = c_sDefaultKeyBindings.find(sRole);
  if (c_sDefaultKeyBindings.end() != it && IsAllowedToOverwriteKeyBinding(sRole))
  {
    QString sOldBinding = m_spSettings->value(CSettings::c_sSettingKeyBindings + sRole,
                                              it->second.toString()).toString();
    if (sOldBinding != sKeySequence.toString())
    {
      m_spSettings->setValue(CSettings::c_sSettingKeyBindings + sRole, sKeySequence.toString());
      emit keyBindingsChanged();
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CSettings::SetMuted(bool bValue)
{
  QMutexLocker locker(&m_settingsMutex);

  bool bMuted = m_spSettings->value(CSettings::c_sSettingMuted).toBool();

  if (bMuted == bValue) { return; }

  m_spSettings->setValue(CSettings::c_sSettingMuted, bValue);

  emit mutedChanged();
}

//----------------------------------------------------------------------------------------
//
bool CSettings::Muted()
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(CSettings::c_sSettingMuted).toBool();
}

//----------------------------------------------------------------------------------------
//
void CSettings::SetOffline(bool bValue)
{
  QMutexLocker locker(&m_settingsMutex);

  bool bOffline = m_spSettings->value(CSettings::c_sSettingOffline).toBool();

  if (bOffline == bValue) { return; }

  m_spSettings->setValue(CSettings::c_sSettingOffline, bValue);

  emit offlineChanged();
}

//----------------------------------------------------------------------------------------
//
bool CSettings::Offline()
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(CSettings::c_sSettingOffline).toBool();
}

//----------------------------------------------------------------------------------------
//
void CSettings::SetResolution(const QSize& size)
{
  QMutexLocker locker(&m_settingsMutex);
  QSize oldValue = m_spSettings->value(CSettings::c_sSettingResolution).toSize();

  if (size == oldValue) { return; }

  m_spSettings->setValue(CSettings::c_sSettingResolution, size);

  emit resolutionChanged();
}

//----------------------------------------------------------------------------------------
//
QSize CSettings::Resolution()
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(CSettings::c_sSettingResolution).toSize();
}

//----------------------------------------------------------------------------------------
//
void CSettings::SetStyle(const QString& sStyle)
{
  QMutexLocker locker(&m_settingsMutex);
  QString sOldStyle = m_spSettings->value(CSettings::c_sSettingStyle).toString();

  if (sOldStyle == sStyle) { return; }

  m_spSettings->setValue(CSettings::c_sSettingStyle, sStyle);

  emit styleChanged();
}

//----------------------------------------------------------------------------------------
//
QString CSettings::Style()
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(CSettings::c_sSettingStyle).toString();
}

//----------------------------------------------------------------------------------------
//
QUrl CSettings::styleFolder()
{
  QMutexLocker locker(&m_settingsMutex);
  QString sStyle = m_spSettings->value(CSettings::c_sSettingStyle).toString();
  QFileInfo info(QLibraryInfo::location(QLibraryInfo::PrefixPath) +
                 QDir::separator() + joip_style::c_sStyleFolder +
                 QDir::separator() + sStyle);
  if (info.exists())
  {
    return QUrl::fromLocalFile(info.absoluteFilePath());
  }
  else
  {
    return QUrl();
  }
}

//----------------------------------------------------------------------------------------
//
QUrl CSettings::styleFolderQml()
{
  QMutexLocker locker(&m_settingsMutex);
  QString sStyle = m_spSettings->value(CSettings::c_sSettingStyle).toString();
  QFileInfo info(QLibraryInfo::location(QLibraryInfo::PrefixPath) +
                 QDir::separator() + joip_style::c_sStyleFolder +
                 QDir::separator() + sStyle +
                 QDir::separator() + joip_style::c_sQmlStyleSubFolder);
  if (info.exists())
  {
    return QUrl::fromLocalFile(info.absoluteFilePath());
  }
  else
  {
    return QUrl();
  }
}

//----------------------------------------------------------------------------------------
//
qint32 CSettings::Version()
{
  SVersion version(VERSION_XYZ);
  return QT_VERSION_CHECK(version.m_iMajor, version.m_iMinor, version.m_iPatch);
}

//----------------------------------------------------------------------------------------
//
void CSettings::SetVolume(double dVolume)
{
  QMutexLocker locker(&m_settingsMutex);
  double dOldValue = m_spSettings->value(CSettings::c_sSettingVolume).toDouble();

  if (qFuzzyCompare(dVolume, dOldValue)) { return; }

  m_spSettings->setValue(CSettings::c_sSettingVolume, dVolume);

  emit volumeChanged();
}

//----------------------------------------------------------------------------------------
//
double CSettings::Volume()
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(CSettings::c_sSettingVolume).toDouble();
}

//----------------------------------------------------------------------------------------
//
bool CSettings::IsAllowedToOverwriteKeyBinding(const QString& sRole)
{
  if (sRole == "Cut" ||
      sRole == "Copy" ||
      sRole == "Pase" ||
      sRole == "Undo" ||
      sRole == "Redo" ||
      sRole == "Search" ||
      sRole == "Backward" ||
      sRole == "Foreward")
  {
    return false;
  }
  return true;
}

//----------------------------------------------------------------------------------------
//
void CSettings::GenerateSettingsIfNotExists()
{
  QMutexLocker locker(&m_settingsMutex);
  bool bNeedsSynch = false;

  // check content path
  if (!m_spSettings->contains(CSettings::c_sSettingContentFolder))
  {
    bNeedsSynch = true;
    QString sContentPath = QCoreApplication::instance()->applicationDirPath() +
        QDir::separator() + ".." + QDir::separator() + "data";
    QFileInfo contentFileInfo(sContentPath);
    m_spSettings->setValue(CSettings::c_sSettingContentFolder,
                           contentFileInfo.absoluteFilePath());

    if (!contentFileInfo.exists())
    {
      QDir::current().mkdir("data");
    }
  }

  // check fullscreen
  if (!m_spSettings->contains(CSettings::c_sSettingFont))
  {
    bNeedsSynch = true;
    m_spSettings->setValue(CSettings::c_sSettingFont, "Arial");
  }

  // check fullscreen
  if (!m_spSettings->contains(CSettings::c_sSettingFullscreen))
  {
    bNeedsSynch = true;
    m_spSettings->setValue(CSettings::c_sSettingFullscreen, true);
  }

  // Keybindings
  for (const QString& sKeyBindingIdx : qAsConst(c_vsKeyBindings))
  {
    auto it = c_sDefaultKeyBindings.find(sKeyBindingIdx);
    if (!m_spSettings->contains(CSettings::c_sSettingKeyBindings + sKeyBindingIdx) &&
        c_sDefaultKeyBindings.end() != it)
    {
      m_spSettings->setValue(CSettings::c_sSettingKeyBindings + sKeyBindingIdx,
                             it->second.toString());
    }
  }

  // check resolution
  if (!m_spSettings->contains(CSettings::c_sSettingResolution))
  {
    bNeedsSynch = true;
    QSize size = QSize(800, 450);
    m_spSettings->setValue(CSettings::c_sSettingResolution, size);
  }

  // check muted
  if (!m_spSettings->contains(CSettings::c_sSettingMuted))
  {
    bNeedsSynch = true;
    m_spSettings->setValue(CSettings::c_sSettingMuted, false);
  }

  // check offline
  if (!m_spSettings->contains(CSettings::c_sSettingOffline))
  {
    bNeedsSynch = true;
    m_spSettings->setValue(CSettings::c_sSettingOffline, false);
  }

  // check style
  if (!m_spSettings->contains(CSettings::c_sSettingStyle))
  {
    bNeedsSynch = true;
    m_spSettings->setValue(CSettings::c_sSettingStyle, "Blue Night");
  }

  // check volume
  if (!m_spSettings->contains(CSettings::c_sSettingVolume))
  {
    bNeedsSynch = true;
    m_spSettings->setValue(CSettings::c_sSettingVolume, 1.0);
  }

  // write file if nesseccary
  if (bNeedsSynch)
  {
    m_spSettings->sync();
  }
}
