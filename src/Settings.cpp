#include "Settings.h"
#include "Application.h"
#include "Style.h"
#include "SVersion.h"

#include "Systems/DatabaseInterface/ProjectData.h"

#include "Utils/MetronomeHelpers.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QMutexLocker>
#include <QSettings>
#include <QScreen>
#include <QStandardPaths>
#include <map>

namespace {
  QStringList c_vsKeyBindings = {
    "Pause",
    "Skip",
    "Save",
    "Export",
    "Help",
    "Download",
    "Devices",
    "Exit",
    "Tools",
    "ToggleUI",

    "Answer_1",
    "Answer_2",
    "Answer_3",
    "Answer_4",
    "Answer_5",
    "Answer_6",
    "Answer_7",
    "Answer_8",
    "Answer_9",

    "Notification_1",
    "Notification_2",
    "Notification_3",
    "Notification_4",
    "Notification_5",

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
    "LeftTab_Sequence",
    "LeftTab_Dialogue",

    "RightTab_Resource",
    "RightTab_MediaPlayer",
    "RightTab_Settings",
    "RightTab_Nodes",
    "RightTab_Code",
    "RightTab_Sequence",
    "RightTab_Dialogue",

    "Debug",

    "Cut",
    "Copy",
    "Pase",
    "Undo",
    "Redo",
    "Search",
    "Backward",
    "Forward"
  };
}

//----------------------------------------------------------------------------------------
//
const QString CSettings::c_sVersion = "General/version";
const QString CSettings::c_sSettingAutoPauseInactive = "Content/pauseinactive";
const QString CSettings::c_sSettingAutoUpdate = "General/autoUpdate";
const QString CSettings::c_sSettingCodeEditorCaseInsensitiveSearch = "Content/editorCaseInsensitiveSearch";
const QString CSettings::c_sSettingCodeEditorFont = "Content/editorFont";
const QString CSettings::c_sSettingCodeEditorShowWhitespace = "Content/editorShowWhitespace";
const QString CSettings::c_sSettingConnectToHWOnStartup = "Devices/connectOnStartup";
const QString CSettings::c_sSettingCodeEditorTheme = "Content/editorTheme";
const QString CSettings::c_sSettingContentFolder = "Content/folder";
const QString CSettings::c_sSettingDebugOverlayEnabled = "Debug/debugOverlayEnabled";
const QString CSettings::c_sSettingDominantHand  = "Content/dominantHand";
const QString CSettings::c_sSettingEditorLayout = "Content/preferededitorlayout";
const QString CSettings::c_sSettingFont = "Graphics/font";
const QString CSettings::c_sSettingFullscreen = "Graphics/fullscreen";
const QString CSettings::c_sSettingHideTextbox = "Content/hideTextBox";
const QString CSettings::c_sSettingKeyBindings = "KeyBindings/";
const QString CSettings::c_sSettingMetronomeDefCommands = "Devices/metronomeDefCmd";
const QString CSettings::c_sSettingMetronomeSfx = "Audio/metronomeSfx";
const QString CSettings::c_sSettingMetronomeSizeRel = "Content/metronomeMinSize";
const QString CSettings::c_sSettingMetronomeSizeMin = "Content/metronomeRelSize";
const QString CSettings::c_sSettingMetronomeVolume = "Audio/metronomeVolume";
const QString CSettings::c_sSettingMuted = "Audio/muted";
const QString CSettings::c_sSettingOffline = "Content/offline";
const QString CSettings::c_sSettingPlayerAntialiasing = "Graphics/playerAntialiasing";
const QString CSettings::c_sSettingPlayerDropShadow = "Graphics/playerDropShadow";
const QString CSettings::c_sSettingPlayerImageMipMap = "Graphics/playerMipMap";
const QString CSettings::c_sSettingPlayerImageSmooth = "Graphics/playerSmooth";
const QString CSettings::c_sSettingPushNotifications = "Content/pushnotifications";
const QString CSettings::c_sSettingResolution = "Graphics/resolution";
const QString CSettings::c_sSettingStyle = "Graphics/style";
const QString CSettings::c_sSettingStyleHotLoad = "Debug/stylehotload";
const QString CSettings::c_sSettingVolume = "Audio/volume";
const QString CSettings::c_sWindowMode = "Graphics/windowMode";

const QString CSettings::c_sOrganisation = "Private";
const QString CSettings::c_sApplicationName = "JOIPEngine";

//----------------------------------------------------------------------------------------
//
QString settings::GetSettingsPath()
{
  QSettings s(QSettings::IniFormat, QSettings::UserScope,
              CSettings::c_sOrganisation, CSettings::c_sApplicationName);
  return s.fileName();
}

//----------------------------------------------------------------------------------------
//
CSettings::CSettings(QObject* pParent) :
  QObject (pParent),
  m_settingsMutex(QMutex::Recursive),
  m_spSettings(std::make_shared<QSettings>(QSettings::IniFormat, QSettings::UserScope,
     CSettings::c_sOrganisation, CSettings::c_sApplicationName)),
  m_bOldVersionSaved(false),
  c_sDefaultKeyBindings({
      { "Pause",      QKeySequence(Qt::Key_P)                        },
      { "Skip",       QKeySequence(Qt::Key_Space)                    },
      { "Save",       QKeySequence(QKeySequence::Save)               },
      { "Export",     QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_S) },
      { "Help",       QKeySequence(Qt::Key_H)                        },
      { "Download",   QKeySequence(Qt::Key_D)                        },
      { "Devices",    QKeySequence(Qt::CTRL + Qt::Key_D)             },
      { "Exit",       QKeySequence(Qt::Key_Escape)                   },
      { "Tools",      QKeySequence(Qt::CTRL + Qt::Key_T)             },
      { "ToggleUI",   QKeySequence(Qt::Key_T)                        },

      { "Answer_1",   QKeySequence(Qt::Key_1)            },
      { "Answer_2",   QKeySequence(Qt::Key_2)            },
      { "Answer_3",   QKeySequence(Qt::Key_3)            },
      { "Answer_4",   QKeySequence(Qt::Key_4)            },
      { "Answer_5",   QKeySequence(Qt::Key_5)            },
      { "Answer_6",   QKeySequence(Qt::Key_6)            },
      { "Answer_7",   QKeySequence(Qt::Key_7)            },
      { "Answer_8",   QKeySequence(Qt::Key_8)            },
      { "Answer_9",   QKeySequence(Qt::Key_9)            },

      { "Notification_1", QKeySequence(Qt::Key_C)        },
      { "Notification_2", QKeySequence(Qt::Key_V)        },
      { "Notification_3", QKeySequence(Qt::Key_B)        },
      { "Notification_4", QKeySequence(Qt::Key_N)        },
      { "Notification_5", QKeySequence(Qt::Key_M)        },

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
      { "LeftTab_Sequence",     QKeySequence(Qt::CTRL + Qt::Key_P) },
      { "LeftTab_Dialogue",     QKeySequence(Qt::CTRL + Qt::Key_I) },

      { "RightTab_Resource",    QKeySequence(Qt::ALT  + Qt::Key_R) },
      { "RightTab_MediaPlayer", QKeySequence(Qt::ALT  + Qt::Key_M) },
      { "RightTab_Settings",    QKeySequence(Qt::ALT  + Qt::Key_T) },
      { "RightTab_Nodes",       QKeySequence(Qt::ALT  + Qt::Key_N) },
      { "RightTab_Code",        QKeySequence(Qt::ALT  + Qt::Key_D) },
      { "RightTab_Sequence",    QKeySequence(Qt::ALT  + Qt::Key_P) },
      { "RightTab_Dialogue",    QKeySequence(Qt::ALT  + Qt::Key_I) },

      { "Debug",      QKeySequence(Qt::Key_section) },

      { "Cut",        QKeySequence(QKeySequence::Cut)    },
      { "Copy",       QKeySequence(QKeySequence::Copy)   },
      { "Pase",       QKeySequence(QKeySequence::Paste)  },
      { "Undo",       QKeySequence(QKeySequence::Undo)   },
      { "Redo",       QKeySequence(QKeySequence::Redo)   },
      { "Search",     QKeySequence(QKeySequence::Find)   },
      { "Backward",   QKeySequence(QKeySequence::Back)   },
      { "Forward",    QKeySequence(QKeySequence::Forward)}
    })
{
  qRegisterMetaType<CSettings::EditorType>();

  if (!m_spSettings->contains(CSettings::c_sVersion))
  {
    m_bOldVersionSaved = true;
  }
  else
  {
    quint32 iVersion = SettingsVersion();
    SVersion version(VERSION_XYZ);
    if (QT_VERSION_CHECK(version.m_iMajor, version.m_iMinor, version.m_iPatch) > iVersion)
    {
       m_bOldVersionSaved = true;
    }
  }
  GenerateSettingsIfNotExists();
}

//----------------------------------------------------------------------------------------
//
CSettings::~CSettings()
{

}

//----------------------------------------------------------------------------------------
//
QString CSettings::FileName() const
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->fileName();
}

//----------------------------------------------------------------------------------------
//
bool CSettings::HasRaw(const QString& sSetting)
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->contains(sSetting);
}

//----------------------------------------------------------------------------------------
//
QVariant CSettings::ReadRaw(const QString& sSetting, const QVariant& sDefaultValue)
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(sSetting, sDefaultValue);
}

//----------------------------------------------------------------------------------------
//
void CSettings::WriteRaw(const QString& sSetting, const QVariant& value)
{
  QMutexLocker locker(&m_settingsMutex);
  m_spSettings->setValue(sSetting, value);
}

//----------------------------------------------------------------------------------------
//
void CSettings::SetAutoUpdate(bool bValue)
{
  QMutexLocker locker(&m_settingsMutex);
  bool bAutoUpdate = m_spSettings->value(CSettings::c_sSettingAutoUpdate).toBool();

  if (bAutoUpdate == bValue) { return; }

  m_spSettings->setValue(CSettings::c_sSettingAutoUpdate, bValue);

  emit autoUpdateChanged();
}

//----------------------------------------------------------------------------------------
//
bool CSettings::AutoUpdate()
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(CSettings::c_sSettingAutoUpdate).toBool();
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
#if defined(Q_OS_ANDROID)
  return m_spSettings->value(CSettings::c_sSettingContentFolder).toString();
#else
  QMutexLocker locker(&m_settingsMutex);
  QString sPath = m_spSettings->value(CSettings::c_sSettingContentFolder).toString();
  QFileInfo infoPath(sPath);
  sPath = infoPath.absoluteFilePath();
  if (sPath.lastIndexOf("/") == sPath.size() - 1)
  {
    sPath = sPath.left(sPath.size()-1);
  }
  return sPath;
#endif
}

//----------------------------------------------------------------------------------------
//
void CSettings::SetConnectToHWOnStartup(bool bValue)
{
  QMutexLocker locker(&m_settingsMutex);
  bool bOldValue = m_spSettings->value(CSettings::c_sSettingConnectToHWOnStartup).toBool();

  if (bOldValue == bValue) { return; }

  m_spSettings->setValue(CSettings::c_sSettingConnectToHWOnStartup, bValue);

  emit connectToHWOnStartupChanged();
}

//----------------------------------------------------------------------------------------
//
bool CSettings::ConnectToHWOnStartup() const
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(CSettings::c_sSettingConnectToHWOnStartup).toBool();
}

//----------------------------------------------------------------------------------------
//
void CSettings::SetDebugOverlayEnabled(bool value)
{
  QMutexLocker locker(&m_settingsMutex);
  bool bOldValue = m_spSettings->value(CSettings::c_sSettingDebugOverlayEnabled).toBool();

  if (bOldValue == value) { return; }

  m_spSettings->setValue(CSettings::c_sSettingDebugOverlayEnabled, value);

  emit debugOverlayEnabledChanged();
}

//----------------------------------------------------------------------------------------
//
bool CSettings::DebugOverlayEnabled() const
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(CSettings::c_sSettingDebugOverlayEnabled).toBool();
}

//----------------------------------------------------------------------------------------
//
DominantHand::EDominantHand CSettings::GetDominantHand() const
{
  QMutexLocker locker(&m_settingsMutex);
  return static_cast<DominantHand::EDominantHand>(
      m_spSettings->value(CSettings::c_sSettingDominantHand).toInt());
}

//----------------------------------------------------------------------------------------
//
void CSettings::SetDominantHand(DominantHand::EDominantHand hand)
{
  QMutexLocker locker(&m_settingsMutex);
  DominantHand::EDominantHand oldValue =
      static_cast<DominantHand::EDominantHand>(
          m_spSettings->value(CSettings::c_sSettingDominantHand).toInt());

  if (oldValue == hand) { return; }

  m_spSettings->setValue(CSettings::c_sSettingDominantHand, static_cast<qint32>(hand));

  emit dominantHandChanged();
}

//----------------------------------------------------------------------------------------
//
void CSettings::SetEditorCaseInsensitiveSearch(bool bValue)
{
  QMutexLocker locker(&m_settingsMutex);
  bool bOldValue = m_spSettings->value(CSettings::c_sSettingCodeEditorCaseInsensitiveSearch).toBool();

  if (bOldValue == bValue) { return; }

  m_spSettings->setValue(CSettings::c_sSettingCodeEditorCaseInsensitiveSearch, bValue);

  emit editorCaseInsensitiveSearchChanged();
}

//----------------------------------------------------------------------------------------
//
bool CSettings::EditorCaseInsensitiveSearch() const
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(CSettings::c_sSettingCodeEditorCaseInsensitiveSearch).toBool();
}

//----------------------------------------------------------------------------------------
//
void CSettings::SetEditorFont(const QString& sValue)
{
  QMutexLocker locker(&m_settingsMutex);
  QString sOldFont = m_spSettings->value(CSettings::c_sSettingCodeEditorFont).toString();

  if (sOldFont == sValue) { return; }

  m_spSettings->setValue(CSettings::c_sSettingCodeEditorFont, sValue);

  emit editorFontChanged();
}

//----------------------------------------------------------------------------------------
//
QString CSettings::EditorFont() const
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(CSettings::c_sSettingCodeEditorFont).toString();
}

//----------------------------------------------------------------------------------------
//
void CSettings::SetEditorShowWhitespace(bool bValue)
{
  QMutexLocker locker(&m_settingsMutex);
  bool bOldValue = m_spSettings->value(CSettings::c_sSettingCodeEditorShowWhitespace).toBool();

  if (bOldValue == bValue) { return; }

  m_spSettings->setValue(CSettings::c_sSettingCodeEditorShowWhitespace, bValue);

  emit editorShowWhitespaceChanged();
}

//----------------------------------------------------------------------------------------
//
bool CSettings::EditorShowWhitespace() const
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(CSettings::c_sSettingCodeEditorShowWhitespace).toBool();
}

//----------------------------------------------------------------------------------------
//
void CSettings::SetEditorTheme(const QString& sValue)
{
  QMutexLocker locker(&m_settingsMutex);
  QString sOldStyle = m_spSettings->value(CSettings::c_sSettingCodeEditorTheme).toString();

  if (sOldStyle == sValue) { return; }

  m_spSettings->setValue(CSettings::c_sSettingCodeEditorTheme, sValue);

  emit editorThemeChanged();
}

//----------------------------------------------------------------------------------------
//
QString CSettings::EditorTheme() const
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(CSettings::c_sSettingCodeEditorTheme).toString();
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
#if defined(Q_OS_ANDROID)
  Q_UNUSED(bValue)
#else
  WindowMode oldMode =  GetWindowMode();
  WindowMode newMode = bValue ? WindowMode::eFullscreen : WindowMode ::eBorderless;

  if (oldMode == newMode) { return; }

  SetWindowMode(newMode);

  emit fullscreenChanged();
#endif
}

//----------------------------------------------------------------------------------------
//
bool CSettings::Fullscreen()
{
#if defined(Q_OS_ANDROID)
  return true;
#else
  return GetWindowMode() == WindowMode::eFullscreen;
#endif
}

//----------------------------------------------------------------------------------------
//
void CSettings::SetHideSettingsTimeout(int iValue)
{
  QMutexLocker locker(&m_settingsMutex);

  qint32 iOldVal = m_spSettings->value(CSettings::c_sSettingHideTextbox).toInt();

  if (iOldVal == iValue) { return; }

  m_spSettings->setValue(CSettings::c_sSettingHideTextbox, iValue);

  emit hideSettingsTimeoutChanged();
}

//----------------------------------------------------------------------------------------
//
int CSettings::HideSettingsTimeout() const
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(CSettings::c_sSettingHideTextbox).toInt();
}

//----------------------------------------------------------------------------------------
//
QStringList CSettings::KeyBindings()
{
#if defined(Q_OS_ANDROID)
  return QStringList();
#else
  return c_vsKeyBindings;
#endif
}

//----------------------------------------------------------------------------------------
//
QKeySequence CSettings::keyBinding(const QString& sRole)
{
#if defined(Q_OS_ANDROID)
  Q_UNUSED(sRole)
  return QKeySequence();
#else
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
#endif
}

//----------------------------------------------------------------------------------------
//
void CSettings::setKeyBinding(const QKeySequence& sKeySequence, const QString& sRole)
{
#if defined(Q_OS_ANDROID)
  Q_UNUSED(sKeySequence)
  Q_UNUSED(sRole)
#else
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
#endif
}

//----------------------------------------------------------------------------------------
//
void CSettings::SetMetronomeDefCommands(int iValue)
{
  QMutexLocker locker(&m_settingsMutex);

  qint32 iOldVal = m_spSettings->value(CSettings::c_sSettingMetronomeDefCommands).toInt();

  if (iOldVal == iValue) { return; }

  m_spSettings->setValue(CSettings::c_sSettingMetronomeDefCommands, iValue);

  emit metronomeDefaultCommandsChanged();
}

//----------------------------------------------------------------------------------------
//
int CSettings::MetronomeDefCommands() const
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(CSettings::c_sSettingMetronomeDefCommands).toInt();
}

//----------------------------------------------------------------------------------------
//
void CSettings::SetMetronomeSfx(const QString& sValue)
{
  QMutexLocker locker(&m_settingsMutex);

  QString sResource = m_spSettings->value(CSettings::c_sSettingMetronomeSfx).toString();

  if (sResource == sValue) { return; }

  m_spSettings->setValue(CSettings::c_sSettingMetronomeSfx, sValue);

  emit metronomeSfxChanged();
}

//----------------------------------------------------------------------------------------
//
QString CSettings::MetronomeSfx() const
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(CSettings::c_sSettingMetronomeSfx).toString();
}

//----------------------------------------------------------------------------------------
//
void CSettings::SetMetronomeSizeRel(double dValue)
{
  QMutexLocker locker(&m_settingsMutex);

  double dVolume = m_spSettings->value(CSettings::c_sSettingMetronomeSizeRel).toDouble();

  if (dVolume == dValue) { return; }

  m_spSettings->setValue(CSettings::c_sSettingMetronomeSizeRel, dValue);

  emit metronomeSizeRelChanged();
}

//----------------------------------------------------------------------------------------
//
double CSettings::MetronomeSizeRel() const
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(CSettings::c_sSettingMetronomeSizeRel).toDouble();
}

//----------------------------------------------------------------------------------------
//
void CSettings::SetMetronomeSizeMin(int iValue)
{
  QMutexLocker locker(&m_settingsMutex);

  int iOldVal = m_spSettings->value(CSettings::c_sSettingMetronomeSizeMin).toInt();

  if (iOldVal == iValue) { return; }

  m_spSettings->setValue(CSettings::c_sSettingMetronomeSizeMin, iValue);

  emit metronomeSizeMinChanged();
}

//----------------------------------------------------------------------------------------
//
int CSettings::MetronomeSizeMin() const
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(CSettings::c_sSettingMetronomeSizeMin).toInt();
}

//----------------------------------------------------------------------------------------
//
void CSettings::SetMetronomeVolume(double dValue)
{
  QMutexLocker locker(&m_settingsMutex);

  double dVolume = m_spSettings->value(CSettings::c_sSettingMetronomeVolume).toDouble();

  if (dVolume == dValue) { return; }

  m_spSettings->setValue(CSettings::c_sSettingMetronomeVolume, dValue);

  emit metronomeVolumeChanged();
}

//----------------------------------------------------------------------------------------
//
double CSettings::MetronomeVolume() const
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(CSettings::c_sSettingMetronomeVolume).toDouble();
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
void CSettings::SetPauseWhenInactive(bool bValue)
{
  QMutexLocker locker(&m_settingsMutex);

  bool bOffline = m_spSettings->value(CSettings::c_sSettingAutoPauseInactive).toBool();

  if (bOffline == bValue) { return; }

  m_spSettings->setValue(CSettings::c_sSettingAutoPauseInactive, bValue);

  emit pauseWhenInactiveChanged();
}

//----------------------------------------------------------------------------------------
//
bool CSettings::PauseWhenInactive()
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(CSettings::c_sSettingAutoPauseInactive).toBool();
}

//----------------------------------------------------------------------------------------
//
void CSettings::SetPlayerAntialiasing(bool bValue)
{
  QMutexLocker locker(&m_settingsMutex);

  bool vSetValue = m_spSettings->value(CSettings::c_sSettingPlayerAntialiasing).toBool();

  if (vSetValue == bValue) { return; }

  m_spSettings->setValue(CSettings::c_sSettingPlayerAntialiasing, bValue);

  emit playerAntialiasingChanged();
}

//----------------------------------------------------------------------------------------
//
bool CSettings::PlayerAntialiasing() const
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(CSettings::c_sSettingPlayerAntialiasing).toBool();
}

//----------------------------------------------------------------------------------------
//
void CSettings::SetPlayerDropShadow(bool bValue)
{
  QMutexLocker locker(&m_settingsMutex);

  bool vSetValue = m_spSettings->value(CSettings::c_sSettingPlayerDropShadow).toBool();

  if (vSetValue == bValue) { return; }

  m_spSettings->setValue(CSettings::c_sSettingPlayerDropShadow, bValue);

  emit playerDropShadowChanged();
}

//----------------------------------------------------------------------------------------
//
bool CSettings::PlayerDropShadow() const
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(CSettings::c_sSettingPlayerDropShadow).toBool();
}

//----------------------------------------------------------------------------------------
//
void CSettings::SetPlayerImageMipMap(bool bValue)
{
  QMutexLocker locker(&m_settingsMutex);

  bool vSetValue = m_spSettings->value(CSettings::c_sSettingPlayerImageMipMap).toBool();

  if (vSetValue == bValue) { return; }

  m_spSettings->setValue(CSettings::c_sSettingPlayerImageMipMap, bValue);

  emit playerImageMipMapChanged();
}

//----------------------------------------------------------------------------------------
//
bool CSettings::PlayerImageMipMap() const
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(CSettings::c_sSettingPlayerImageMipMap).toBool();
}

//----------------------------------------------------------------------------------------
//
void CSettings::SetPlayerImageSmooth(bool bValue)
{
  QMutexLocker locker(&m_settingsMutex);

  bool vSetValue = m_spSettings->value(CSettings::c_sSettingPlayerImageSmooth).toBool();

  if (vSetValue == bValue) { return; }

  m_spSettings->setValue(CSettings::c_sSettingPlayerImageSmooth, bValue);

  emit playerImageSmoothChanged();
}

//----------------------------------------------------------------------------------------
//
bool CSettings::PlayerImageSmooth() const
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(CSettings::c_sSettingPlayerImageSmooth).toBool();
}

//----------------------------------------------------------------------------------------
//
void CSettings::SetPreferedEditorLayout(const EditorType& eType)
{
  QMutexLocker locker(&m_settingsMutex);

  CSettings::EditorType value = static_cast<CSettings::EditorType>(
        m_spSettings->value(CSettings::c_sSettingEditorLayout).toInt());

  if (value == eType) { return; }

  m_spSettings->setValue(CSettings::c_sSettingEditorLayout, static_cast<qint32>(eType));

  emit preferedEditorLayoutChanged();
}

//----------------------------------------------------------------------------------------
//
CSettings::EditorType CSettings::PreferedEditorLayout()
{
  QMutexLocker locker(&m_settingsMutex);
  return static_cast<CSettings::EditorType>(
        m_spSettings->value(CSettings::c_sSettingEditorLayout).toInt());
}

//----------------------------------------------------------------------------------------
//
void CSettings::SetResolution(const QSize& size)
{
#if defined(Q_OS_ANDROID)
  Q_UNUSED(size)
#else
  QMutexLocker locker(&m_settingsMutex);
  QSize oldValue = m_spSettings->value(CSettings::c_sSettingResolution).toSize();

  if (size == oldValue) { return; }

  m_spSettings->setValue(CSettings::c_sSettingResolution, size);

  emit resolutionChanged();
#endif
}

//----------------------------------------------------------------------------------------
//
QString CSettings::Platform() const
{
#if defined(Q_OS_ANDROID)
  return "Android";
#elif defined(Q_OS_WINDOWS)
  return "Windows";
#elif defined(Q_OS_LINUX)
  return "Linux";
#else
  return QString();
#endif
}

//----------------------------------------------------------------------------------------
//
bool CSettings::PushNotifications() const
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(CSettings::c_sSettingPushNotifications).toBool();
}

//----------------------------------------------------------------------------------------
//
void CSettings::SetPushNotifications(bool bValue)
{
  QMutexLocker locker(&m_settingsMutex);

  bool value = static_cast<CSettings::EditorType>(
        m_spSettings->value(CSettings::c_sSettingPushNotifications).toInt());

  if (value == bValue) { return; }

  m_spSettings->setValue(CSettings::c_sSettingPushNotifications, bValue);

  emit pushNotificationsChanged();
}

//----------------------------------------------------------------------------------------
//
QSize CSettings::Resolution()
{
#if defined(Q_OS_ANDROID)
  return QGuiApplication::screens()[0]->geometry().size();
#else
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(CSettings::c_sSettingResolution).toSize();
#endif
}

//----------------------------------------------------------------------------------------
//
bool CSettings::HasOldSettingsVersion()
{
  QMutexLocker locker(&m_settingsMutex);
  return m_bOldVersionSaved;
}

//----------------------------------------------------------------------------------------
//
void CSettings::SetSettingsVersion(quint32 iVersion)
{
  QMutexLocker locker(&m_settingsMutex);

  quint32 iOldVersion = m_spSettings->value(CSettings::c_sVersion).toUInt();

  if (iOldVersion == iVersion) { return; }

  m_spSettings->setValue(CSettings::c_sVersion, iVersion);
  m_bOldVersionSaved = false;
}

//----------------------------------------------------------------------------------------
//
quint32 CSettings::SettingsVersion()
{
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(CSettings::c_sVersion).toUInt();
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
void CSettings::SetStyleHotLoad(bool bValue)
{
#if defined(Q_OS_ANDROID)
  Q_UNUSED(bValue)
#else
  QMutexLocker locker(&m_settingsMutex);
  bool iOldValue = m_spSettings->value(CSettings::c_sSettingStyleHotLoad).toBool();

  if (iOldValue == bValue) { return; }

  m_spSettings->setValue(CSettings::c_sSettingStyleHotLoad, bValue);

  emit styleHotLoadChanged();
#endif
}

//----------------------------------------------------------------------------------------
//
bool CSettings::StyleHotLoad()
{
#if defined(Q_OS_ANDROID)
  return false;
#else
  QMutexLocker locker(&m_settingsMutex);
  return m_spSettings->value(CSettings::c_sSettingStyleHotLoad).toBool();
#endif
}

//----------------------------------------------------------------------------------------
//
QUrl CSettings::styleFolder()
{
  QMutexLocker locker(&m_settingsMutex);
  QString sStyle = m_spSettings->value(CSettings::c_sSettingStyle).toString();
  QFileInfo info(joip_style::StyleFolder() +
                 QDir::separator() + sStyle);
  if (info.exists())
  {
    return QUrl::fromLocalFile(info.absoluteFilePath());
  }
  else
  {
    return QUrl(QString(joip_style::c_sDefaultStyleFolder));
  }
}

//----------------------------------------------------------------------------------------
//
QUrl CSettings::styleFolderQml()
{
  QMutexLocker locker(&m_settingsMutex);
  QString sStyle = m_spSettings->value(CSettings::c_sSettingStyle).toString();
  QFileInfo info(joip_style::StyleFolder() +
                 QDir::separator() + sStyle +
                 QDir::separator() + joip_style::c_sQmlStyleSubFolder);
  if (info.exists())
  {
    return QUrl::fromLocalFile(info.absoluteFilePath());
  }
  else
  {
    return QUrl(QString("qrc") + joip_style::c_sDefaultStyleFolder + joip_style::c_sQmlStyleSubFolder);
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
CSettings::WindowMode CSettings::GetWindowMode() const
{
#if defined(Q_OS_ANDROID)
  return CSettings::WindowMode::eFullscreen;
#else
  QMutexLocker locker(&m_settingsMutex);
  return static_cast<CSettings::WindowMode>(m_spSettings->value(CSettings::c_sWindowMode).toInt());
#endif
}

//----------------------------------------------------------------------------------------
//
void CSettings::SetWindowMode(const CSettings::WindowMode& mode)
{
  QMutexLocker locker(&m_settingsMutex);

  quint32 iOldMode = m_spSettings->value(CSettings::c_sWindowMode).toInt();

  if (iOldMode == mode) { return; }

  m_spSettings->setValue(CSettings::c_sWindowMode, static_cast<qint32>(mode));

  emit windowModeChanged();
}

//----------------------------------------------------------------------------------------
//
bool CSettings::IsAllowedToOverwriteKeyBinding(const QString& sRole)
{
#if defined(Q_OS_ANDROID)
  Q_UNUSED(sRole)
  return false;
#else
  if (sRole == "Cut" ||
      sRole == "Copy" ||
      sRole == "Pase" ||
      sRole == "Undo" ||
      sRole == "Redo" ||
      sRole == "Search" ||
      sRole == "Backward" ||
      sRole == "Forward")
  {
    return false;
  }
  return true;
#endif
}

//----------------------------------------------------------------------------------------
//
void CSettings::GenerateSettingsIfNotExists()
{
  QMutexLocker locker(&m_settingsMutex);
  bool bNeedsSynch = false;

  // check offline
  if (!m_spSettings->contains(CSettings::c_sSettingAutoPauseInactive))
  {
    bNeedsSynch = true;
    m_spSettings->setValue(CSettings::c_sSettingAutoPauseInactive, true);
  }

  // check autoupdate
  if (!m_spSettings->contains(CSettings::c_sSettingAutoUpdate))
  {
    bNeedsSynch = true;
    m_spSettings->setValue(CSettings::c_sSettingAutoUpdate, true);
  }

  // check content path
  if (!m_spSettings->contains(CSettings::c_sSettingContentFolder))
  {
    bNeedsSynch = true;
#if defined(Q_OS_ANDROID)
    QString sContentPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/Teases";
    QFileInfo contentFileInfo(sContentPath);
    m_spSettings->setValue(CSettings::c_sSettingContentFolder, sContentPath);
#elif defined(Q_OS_LINUX)
    QString sContentPath = QCoreApplication::instance()->applicationDirPath() +
        QDir::separator() + ".." + QDir::separator() + "data";
    const QString sAppImg = qgetenv("APPIMAGE");
    if (!sAppImg.isEmpty())
    {
      sContentPath = QFileInfo(sAppImg).absolutePath() +
              QDir::separator() + "data";
    }
    QFileInfo contentFileInfo(sContentPath);
    m_spSettings->setValue(CSettings::c_sSettingContentFolder,
                           contentFileInfo.absoluteFilePath());
#else
    QString sContentPath = QCoreApplication::instance()->applicationDirPath() +
        QDir::separator() + ".." + QDir::separator() + "data";
    QFileInfo contentFileInfo(sContentPath);
    m_spSettings->setValue(CSettings::c_sSettingContentFolder,
                           contentFileInfo.absoluteFilePath());
#endif
    if (!contentFileInfo.exists())
    {
      QDir::current().mkdir("data");
    }
  }

  // check code editor settings
  if (!m_spSettings->contains(CSettings::c_sSettingCodeEditorCaseInsensitiveSearch))
  {
    bNeedsSynch = true;
    m_spSettings->setValue(CSettings::c_sSettingCodeEditorCaseInsensitiveSearch, true);
  }
  if (!m_spSettings->contains(CSettings::c_sSettingCodeEditorFont))
  {
    bNeedsSynch = true;
    m_spSettings->setValue(CSettings::c_sSettingCodeEditorFont, "Courier New");
  }
  if (!m_spSettings->contains(CSettings::c_sSettingCodeEditorShowWhitespace))
  {
    bNeedsSynch = true;
    m_spSettings->setValue(CSettings::c_sSettingCodeEditorShowWhitespace, true);
  }
  if (!m_spSettings->contains(CSettings::c_sSettingCodeEditorTheme))
  {
    bNeedsSynch = true;
    m_spSettings->setValue(CSettings::c_sSettingCodeEditorTheme, "");
  }

  if (!m_spSettings->contains(CSettings::c_sSettingDominantHand))
  {
    bNeedsSynch = true;
    m_spSettings->setValue(CSettings::c_sSettingDominantHand,
                           static_cast<qint32>(DominantHand::NoDominantHand));
  }

  // check font
  if (!m_spSettings->contains(CSettings::c_sSettingFont))
  {
    bNeedsSynch = true;
    m_spSettings->setValue(CSettings::c_sSettingFont, "Arial");
  }

  // check editor Layout
  if (!m_spSettings->contains(CSettings::c_sSettingEditorLayout))
  {
    bNeedsSynch = true;
#if defined(Q_OS_ANDROID)
    m_spSettings->setValue(CSettings::c_sSettingEditorLayout, CSettings::eCompact);
#else
    m_spSettings->setValue(CSettings::c_sSettingEditorLayout, CSettings::eNone);
#endif
  }

  // check window mode
  if (!m_spSettings->contains(CSettings::c_sWindowMode))
  {
    if (m_spSettings->contains(CSettings::c_sSettingFullscreen))
    {
      bNeedsSynch = true;
      m_spSettings->setValue(CSettings::c_sWindowMode,
                             m_spSettings->value(CSettings::c_sSettingFullscreen).toBool() ?
                             WindowMode::eFullscreen : WindowMode::eBorderless);
    }
    else
    {
      bNeedsSynch = true;
      m_spSettings->setValue(CSettings::c_sWindowMode, WindowMode::eFullscreen);
    }
  }

  if (!m_spSettings->contains(CSettings::c_sSettingHideTextbox))
  {
    bNeedsSynch = true;
    m_spSettings->setValue(CSettings::c_sSettingHideTextbox, 2000);
  }

  // Keybindings
#if !defined(Q_OS_ANDROID)
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
#endif

  // check metronome commands
  if (!m_spSettings->contains(CSettings::c_sSettingMetronomeDefCommands))
  {
    bNeedsSynch = true;
    m_spSettings->setValue(CSettings::c_sSettingMetronomeDefCommands, EToyMetronomeCommandModeFlag::eDefault);
  }

  // check metronome sfx
  if (!m_spSettings->contains(CSettings::c_sSettingMetronomeSfx))
  {
    bNeedsSynch = true;
    m_spSettings->setValue(CSettings::c_sSettingMetronomeSfx, QString(metronome::c_sSfxToc));
  }

  // check metronome size settings
  if (!m_spSettings->contains(CSettings::c_sSettingMetronomeSizeRel))
  {
    bNeedsSynch = true;
    m_spSettings->setValue(CSettings::c_sSettingMetronomeSizeRel, 0.167);
  }
  if (!m_spSettings->contains(CSettings::c_sSettingMetronomeSizeMin))
  {
    bNeedsSynch = true;
    m_spSettings->setValue(CSettings::c_sSettingMetronomeSizeMin, 100);
  }

  // check metronome volume
  if (!m_spSettings->contains(CSettings::c_sSettingMetronomeVolume))
  {
    bNeedsSynch = true;
    m_spSettings->setValue(CSettings::c_sSettingMetronomeVolume, 1.0);
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

  // check hardware autoconnect setting, depends on offline setting
  if (!m_spSettings->contains(CSettings::c_sSettingConnectToHWOnStartup))
  {
    bNeedsSynch = true;
    if (m_spSettings->contains(CSettings::c_sSettingOffline))
    {
      m_spSettings->setValue(CSettings::c_sSettingConnectToHWOnStartup,
                             !m_spSettings->value(CSettings::c_sSettingOffline).toBool());
    }
    else
    {
      m_spSettings->setValue(CSettings::c_sSettingConnectToHWOnStartup, true);
    }
  }

  // check player graphical settings
  if (!m_spSettings->contains(CSettings::c_sSettingPlayerAntialiasing))
  {
    bNeedsSynch = true;
    m_spSettings->setValue(CSettings::c_sSettingPlayerAntialiasing, true);
  }
  if (!m_spSettings->contains(CSettings::c_sSettingPlayerDropShadow))
  {
    bNeedsSynch = true;
    m_spSettings->setValue(CSettings::c_sSettingPlayerDropShadow, true);
  }
  if (!m_spSettings->contains(CSettings::c_sSettingPlayerImageMipMap))
  {
    bNeedsSynch = true;
    m_spSettings->setValue(CSettings::c_sSettingPlayerImageMipMap, true);
  }
  if (!m_spSettings->contains(CSettings::c_sSettingPlayerImageSmooth))
  {
    bNeedsSynch = true;
    m_spSettings->setValue(CSettings::c_sSettingPlayerImageSmooth, true);
  }

  // check push notifications
  if (!m_spSettings->contains(CSettings::c_sSettingPushNotifications))
  {
    bNeedsSynch = true;
    m_spSettings->setValue(CSettings::c_sSettingPushNotifications, true);
  }

  // check style
  if (!m_spSettings->contains(CSettings::c_sSettingStyle))
  {
    bNeedsSynch = true;
    m_spSettings->setValue(CSettings::c_sSettingStyle, "Blue Night");
  }

  // check debug overlay setting
  if (!m_spSettings->contains(CSettings::c_sSettingDebugOverlayEnabled))
  {
    bNeedsSynch = true;
    m_spSettings->setValue(CSettings::c_sSettingDebugOverlayEnabled, false);
  }

#if !defined(Q_OS_ANDROID)
  // check style HotLoad
  if (!m_spSettings->contains(CSettings::c_sSettingStyleHotLoad))
  {
    bNeedsSynch = true;
    m_spSettings->setValue(CSettings::c_sSettingStyleHotLoad, false);
  }
#endif

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
