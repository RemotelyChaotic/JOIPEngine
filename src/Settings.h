#ifndef CSETTINGS_H
#define CSETTINGS_H

#include <QKeySequence>
#include <QMutex>
#include <QObject>
#include <QSize>
#include <QString>
#include <QUrl>
#include <memory>

class QSettings;

namespace DominantHand
{
  Q_NAMESPACE
  enum EDominantHand
  {
    NoDominantHand    = 0, // if not set yet
    Left              = 1,
    Right             = 2
  };
  Q_ENUM_NS(EDominantHand)
}

class CSettings : public QObject
{
  Q_OBJECT
  Q_PROPERTY(bool autoUpdate READ AutoUpdate WRITE SetAutoUpdate NOTIFY autoUpdateChanged)
  Q_PROPERTY(bool connectToHWOnStartup READ ConnectToHWOnStartup WRITE SetConnectToHWOnStartup NOTIFY connectToHWOnStartupChanged)
  Q_PROPERTY(QString contentFolder READ ContentFolder WRITE SetContentFolder NOTIFY contentFolderChanged)
  Q_PROPERTY(bool debugOverlayEnabled READ DebugOverlayEnabled WRITE SetDebugOverlayEnabled NOTIFY debugOverlayEnabledChanged)
  Q_PROPERTY(DominantHand::EDominantHand dominantHand READ GetDominantHand WRITE SetDominantHand NOTIFY dominantHandChanged)
  Q_PROPERTY(bool editorCaseInsensitiveSearch READ EditorCaseInsensitiveSearch WRITE SetEditorCaseInsensitiveSearch NOTIFY editorCaseInsensitiveSearchChanged)
  Q_PROPERTY(QString editorFont READ EditorFont WRITE SetEditorFont NOTIFY editorFontChanged)
  Q_PROPERTY(bool editorShowWhitespace READ EditorShowWhitespace WRITE SetEditorShowWhitespace NOTIFY editorShowWhitespaceChanged)
  Q_PROPERTY(QString editorTheme READ EditorTheme WRITE SetEditorTheme NOTIFY editorThemeChanged)
  Q_PROPERTY(QString font READ Font WRITE SetFont NOTIFY fontChanged)
  Q_PROPERTY(bool fullscreen READ Fullscreen WRITE SetFullscreen NOTIFY fullscreenChanged)
  Q_PROPERTY(QStringList keyBindings READ KeyBindings CONSTANT)
  Q_PROPERTY(int hideSettingsTimeout READ HideSettingsTimeout WRITE SetHideSettingsTimeout NOTIFY hideSettingsTimeoutChanged)
  Q_PROPERTY(int metronomeDefaultCommands READ MetronomeDefCommands WRITE SetMetronomeDefCommands NOTIFY metronomeDefaultCommandsChanged)
  Q_PROPERTY(QString metronomeSfx READ MetronomeSfx WRITE SetMetronomeSfx NOTIFY metronomeSfxChanged)
  Q_PROPERTY(double metronomeSizeRel READ MetronomeSizeRel WRITE SetMetronomeSizeRel NOTIFY metronomeSizeRelChanged)
  Q_PROPERTY(int metronomeSizeMin READ MetronomeSizeMin WRITE SetMetronomeSizeMin NOTIFY metronomeSizeMinChanged)
  Q_PROPERTY(double metronomeVolume READ MetronomeVolume WRITE SetMetronomeVolume NOTIFY metronomeSizeMinChanged)
  Q_PROPERTY(bool muted READ Muted WRITE SetMuted NOTIFY mutedChanged)
  Q_PROPERTY(bool offline READ Offline WRITE SetOffline NOTIFY offlineChanged)
  Q_PROPERTY(bool pauseWhenInactive READ PauseWhenInactive WRITE SetPauseWhenInactive NOTIFY pauseWhenInactiveChanged)
  Q_PROPERTY(QString platform READ Platform CONSTANT)
  Q_PROPERTY(bool playerAntialiasing READ PlayerAntialiasing WRITE SetPlayerAntialiasing NOTIFY playerAntialiasingChanged)
  Q_PROPERTY(bool playerDropShadow READ PlayerDropShadow WRITE SetPlayerDropShadow NOTIFY playerDropShadowChanged)
  Q_PROPERTY(bool playerImageMipMap READ PlayerImageMipMap WRITE SetPlayerImageMipMap NOTIFY playerImageMipMapChanged)
  Q_PROPERTY(bool playerImageSmooth READ PlayerImageSmooth WRITE SetPlayerImageSmooth NOTIFY playerImageSmoothChanged)
  Q_PROPERTY(EditorType preferedEditorLayout READ PreferedEditorLayout WRITE SetPreferedEditorLayout NOTIFY preferedEditorLayoutChanged)
  Q_PROPERTY(bool pushNotifications READ PushNotifications WRITE SetPushNotifications NOTIFY pushNotificationsChanged)
  Q_PROPERTY(QSize resolution READ Resolution WRITE SetResolution NOTIFY resolutionChanged)
  Q_PROPERTY(QString style READ Style WRITE SetStyle NOTIFY styleChanged)
  Q_PROPERTY(bool styleHotLoad READ StyleHotLoad WRITE SetStyleHotLoad NOTIFY styleHotLoadChanged)
  Q_PROPERTY(qint32 version READ Version CONSTANT)
  Q_PROPERTY(double volume READ Volume WRITE SetVolume NOTIFY volumeChanged)
  Q_PROPERTY(WindowMode windowMode READ GetWindowMode WRITE SetWindowMode NOTIFY windowModeChanged)

public:
  static const QString c_sVersion;
  static const QString c_sSettingAutoPauseInactive;
  static const QString c_sSettingAutoUpdate;
  static const QString c_sSettingCodeEditorCaseInsensitiveSearch;
  static const QString c_sSettingCodeEditorFont;
  static const QString c_sSettingCodeEditorShowWhitespace;
  static const QString c_sSettingCodeEditorTheme;
  static const QString c_sSettingConnectToHWOnStartup;
  static const QString c_sSettingContentFolder;
  static const QString c_sSettingDebugOverlayEnabled;
  static const QString c_sSettingDominantHand;
  static const QString c_sSettingEditorLayout;
  static const QString c_sSettingFont;
  static const QString c_sSettingFullscreen;
  static const QString c_sSettingHideTextbox;
  static const QString c_sSettingKeyBindings;
  static const QString c_sSettingMetronomeDefCommands;
  static const QString c_sSettingMetronomeSfx;
  static const QString c_sSettingMetronomeSizeRel;
  static const QString c_sSettingMetronomeSizeMin;
  static const QString c_sSettingMetronomeVolume;
  static const QString c_sSettingMuted;
  static const QString c_sSettingOffline;
  static const QString c_sSettingPlayerAntialiasing;
  static const QString c_sSettingPlayerDropShadow;
  static const QString c_sSettingPlayerImageMipMap;
  static const QString c_sSettingPlayerImageSmooth;
  static const QString c_sSettingPushNotifications;
  static const QString c_sSettingResolution;
  static const QString c_sSettingStyle;
  static const QString c_sSettingStyleHotLoad;
  static const QString c_sSettingVolume;
  static const QString c_sWindowMode;

  static const QString c_sOrganisation;
  static const QString c_sApplicationName;

  enum EditorType {
    eNone    = 0, // if not set yet
    eClassic = 1,
    eModern  = 2,
    eCompact = 3
  };
  Q_ENUM(EditorType)

  enum WindowMode {
    eFullscreen   = 0,
    eBorderless   = 1,
    eWindowed     = 2
  };
  Q_ENUM(WindowMode)

  explicit CSettings(QObject* pParent = nullptr);
  ~CSettings() override;

  QString FileName() const;
  bool HasRaw(const QString& sSetting);
  QVariant ReadRaw(const QString& sSetting, const QVariant& sDefaultValue);
  void WriteRaw(const QString& sSetting, const QVariant& value);

  void SetAutoUpdate(bool bValue);
  bool AutoUpdate();
  void SetContentFolder(const QString& sPath);
  QString ContentFolder();
  void SetConnectToHWOnStartup(bool bValue);
  bool ConnectToHWOnStartup() const;
  void SetDebugOverlayEnabled(bool value);
  bool DebugOverlayEnabled() const;
  DominantHand::EDominantHand GetDominantHand() const;
  void SetDominantHand(DominantHand::EDominantHand hand);
  void SetEditorCaseInsensitiveSearch(bool bValue);
  bool EditorCaseInsensitiveSearch() const;
  void SetEditorFont(const QString& sValue);
  QString EditorFont() const;
  void SetEditorShowWhitespace(bool bValue);
  bool EditorShowWhitespace() const;
  void SetEditorTheme(const QString& sValue);
  QString EditorTheme() const;
  void SetFont(const QString& sFont);
  QString Font();
  void SetFullscreen(bool bValue);
  bool Fullscreen();
  void SetHideSettingsTimeout(int iValue);
  int HideSettingsTimeout() const;
  QStringList KeyBindings();
  Q_INVOKABLE QKeySequence keyBinding(const QString& sRole);
  Q_INVOKABLE void setKeyBinding(const QKeySequence& sKeySequence, const QString& sRole);
  void SetMetronomeDefCommands(int iValue);
  int MetronomeDefCommands() const;
  void SetMetronomeSfx(const QString& sResource);
  QString MetronomeSfx() const;
  void SetMetronomeSizeRel(double dVal);
  double MetronomeSizeRel() const;
  void SetMetronomeSizeMin(int iValue);
  int MetronomeSizeMin() const;
  void SetMetronomeVolume(double dVolume);
  double MetronomeVolume() const;
  void SetMuted(bool bValue);
  bool Muted();
  void SetOffline(bool bValue);
  bool Offline();
  void SetPauseWhenInactive(bool bValue);
  bool PauseWhenInactive();
  void SetPlayerAntialiasing(bool bValue);
  bool PlayerAntialiasing() const;
  void SetPlayerDropShadow(bool bValue);
  bool PlayerDropShadow() const;
  void SetPlayerImageMipMap(bool bValue);
  bool PlayerImageMipMap() const;
  void SetPlayerImageSmooth(bool bValue);
  bool PlayerImageSmooth() const;
  void SetPreferedEditorLayout(const EditorType& eType);
  EditorType PreferedEditorLayout();
  void SetResolution(const QSize& size);
  QString Platform() const;
  bool PushNotifications() const;
  void SetPushNotifications(bool bValue);
  QSize Resolution();
  bool HasOldSettingsVersion();
  void SetSettingsVersion(quint32 uiVersion);
  quint32 SettingsVersion();
  void SetStyle(const QString& sStyle);
  QString Style();
  void SetStyleHotLoad(bool bValue);
  bool StyleHotLoad();
  Q_INVOKABLE QUrl styleFolder();
  Q_INVOKABLE QUrl styleFolderQml();
  qint32 Version();
  void SetVolume(double dVolume);
  double Volume();
  WindowMode GetWindowMode() const;
  void SetWindowMode(const WindowMode& mode);

  static bool IsAllowedToOverwriteKeyBinding(const QString& sRole);

signals:
  void autoUpdateChanged();
  void contentFolderChanged();
  void connectToHWOnStartupChanged();
  void debugOverlayEnabledChanged();
  void dominantHandChanged();
  void editorCaseInsensitiveSearchChanged();
  void editorFontChanged();
  void editorShowWhitespaceChanged();
  void editorThemeChanged();
  void fontChanged();
  void fullscreenChanged();
  void hideSettingsTimeoutChanged();
  void keyBindingsChanged();
  void metronomeDefaultCommandsChanged();
  void metronomeSfxChanged();
  void metronomeSizeRelChanged();
  void metronomeSizeMinChanged();
  void metronomeVolumeChanged();
  void mutedChanged();
  void offlineChanged();
  void pauseWhenInactiveChanged();
  void playerAntialiasingChanged();
  void playerDropShadowChanged();
  void playerImageMipMapChanged();
  void playerImageSmoothChanged();
  void pushNotificationsChanged();
  void preferedEditorLayoutChanged();
  void resolutionChanged();
  void styleChanged();
  void styleHotLoadChanged();
  void volumeChanged();
  void windowModeChanged();

private:
  void GenerateSettingsIfNotExists();

  mutable QMutex             m_settingsMutex;
  std::shared_ptr<QSettings> m_spSettings;
  bool                       m_bOldVersionSaved;

  const std::map<QString, QKeySequence> c_sDefaultKeyBindings;
};

Q_DECLARE_METATYPE(CSettings*)

#endif // CSETTINGS_H
