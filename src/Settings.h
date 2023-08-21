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

class CSettings : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QString contentFolder READ ContentFolder WRITE SetContentFolder NOTIFY contentFolderChanged)
  Q_PROPERTY(QString font READ Font WRITE SetFont NOTIFY fontChanged)
  Q_PROPERTY(bool fullscreen READ Fullscreen WRITE SetFullscreen NOTIFY fullscreenChanged)
  Q_PROPERTY(QStringList keyBindings READ KeyBindings CONSTANT)
  Q_PROPERTY(QString metronomeSfx READ MetronomeSfx WRITE SetMetronomeSfx NOTIFY metronomeSfxChanged)
  Q_PROPERTY(double metronomeVolume READ MetronomeVolume WRITE SetMetronomeVolume NOTIFY metronomeVolumeChanged)
  Q_PROPERTY(bool muted READ Muted WRITE SetMuted NOTIFY mutedChanged)
  Q_PROPERTY(bool offline READ Offline WRITE SetOffline NOTIFY offlineChanged)
  Q_PROPERTY(bool pauseWhenInactive READ PauseWhenInactive WRITE SetPauseWhenInactive NOTIFY pauseWhenInactiveChanged)
  Q_PROPERTY(QString platform READ Platform CONSTANT)
  Q_PROPERTY(EditorType preferedEditorLayout READ PreferedEditorLayout WRITE SetPreferedEditorLayout NOTIFY preferedEditorLayoutChanged)
  Q_PROPERTY(bool pushNotifications READ PushNotifications WRITE SetPushNotifications NOTIFY pushNotificationsChanged)
  Q_PROPERTY(QSize resolution READ Resolution WRITE SetResolution NOTIFY resolutionChanged)
  Q_PROPERTY(QString style READ Style WRITE SetStyle NOTIFY styleChanged)
  Q_PROPERTY(double styleHotLoad READ StyleHotLoad WRITE SetStyleHotLoad NOTIFY styleHotLoadChanged)
  Q_PROPERTY(qint32 version READ Version CONSTANT)
  Q_PROPERTY(double volume READ Volume WRITE SetVolume NOTIFY volumeChanged)
  Q_PROPERTY(WindowMode windowMode READ GetWindowMode WRITE SetWindowMode NOTIFY windowModeChanged)

public:
  static const QString c_sVersion;
  static const QString c_sSettingAutoPauseInactive;
  static const QString c_sSettingContentFolder;
  static const QString c_sSettingEditorLayout;
  static const QString c_sSettingFont;
  static const QString c_sSettingFullscreen;
  static const QString c_sSettingKeyBindings;
  static const QString c_sSettingMetronomeSfx;
  static const QString c_sSettingMetronomeVolume;
  static const QString c_sSettingMuted;
  static const QString c_sSettingOffline;
  static const QString c_sSettingPushNotifications;
  static const QString c_sSettingResolution;
  static const QString c_sSettingStyle;
  static const QString c_sSettingStyleHotLoad;
  static const QString c_sSettingVolume;
  static const QString c_sWindowMode;

  static const QString c_sOrganisation;
  static const QString c_sApplicationName;

  enum EditorType {
    eNone    = 0,
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

  bool HasRaw(const QString& sSetting);
  QVariant ReadRaw(const QString& sSetting, const QVariant& sDefaultValue);
  void WriteRaw(const QString& sSetting, const QVariant& value);

  void SetContentFolder(const QString& sPath);
  QString ContentFolder();
  void SetFont(const QString& sFont);
  QString Font();
  void SetFullscreen(bool bValue);
  bool Fullscreen();
  QStringList KeyBindings();
  Q_INVOKABLE QKeySequence keyBinding(const QString& sRole);
  Q_INVOKABLE void setKeyBinding(const QKeySequence& sKeySequence, const QString& sRole);
  void SetMetronomeSfx(const QString& sResource);
  QString MetronomeSfx() const;
  void SetMetronomeVolume(double dVolume);
  double MetronomeVolume() const;
  void SetMuted(bool bValue);
  bool Muted();
  void SetOffline(bool bValue);
  bool Offline();
  void SetPauseWhenInactive(bool bValue);
  bool PauseWhenInactive();
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
  void contentFolderChanged();
  void fontChanged();
  void fullscreenChanged();
  void keyBindingsChanged();
  void metronomeSfxChanged();
  void metronomeVolumeChanged();
  void mutedChanged();
  void offlineChanged();
  void pauseWhenInactiveChanged();
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
