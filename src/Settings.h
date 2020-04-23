#ifndef CSETTINGS_H
#define CSETTINGS_H

#include <QKeySequence>
#include <QMutex>
#include <QObject>
#include <QSize>
#include <QString>
#include <memory>

class QSettings;

class CSettings : public QObject {

  Q_OBJECT
  Q_PROPERTY(QString contentFolder READ ContentFolder WRITE SetContentFolder NOTIFY ContentFolderChanged)
  Q_PROPERTY(QString font READ Font WRITE SetFont NOTIFY FontChanged)
  Q_PROPERTY(bool fullscreen READ Fullscreen WRITE SetFullscreen NOTIFY FullscreenChanged)
  Q_PROPERTY(QStringList keyBindings READ KeyBindings)
  Q_PROPERTY(bool muted READ Muted WRITE SetMuted NOTIFY MutedChanged)
  Q_PROPERTY(QSize resolution READ Resolution WRITE SetResolution NOTIFY ResolutionChanged)
  Q_PROPERTY(QString style READ Style WRITE SetStyle NOTIFY StyleChanged)
  Q_PROPERTY(double volume READ Volume WRITE SetVolume NOTIFY VolumeChanged)

public:
  static const QString c_sSettingContentFolder;
  static const QString c_sSettingFont;
  static const QString c_sSettingFullscreen;
  static const QString c_sSettingKeyBindings;
  static const QString c_sSettingMuted;
  static const QString c_sSettingResolution;
  static const QString c_sSettingStyle;
  static const QString c_sSettingVolume;

  static const QString c_sOrganisation;
  static const QString c_sApplicationName;

  explicit CSettings(QObject* pParent = nullptr);
  ~CSettings() override;

  void SetContentFolder(const QString& sPath);
  QString ContentFolder();
  void SetFont(const QString& sFont);
  QString Font();
  void SetFullscreen(bool bValue);
  bool Fullscreen();
  QStringList KeyBindings();
  Q_INVOKABLE QKeySequence KeyBinding(const QString& sRole);
  Q_INVOKABLE void SetKeyBinding(const QKeySequence& sKeySequence, const QString& sRole);
  void SetMuted(bool bValue);
  bool Muted();
  void SetResolution(const QSize& size);
  QSize Resolution();
  void SetStyle(const QString& sStyle);
  QString Style();
  void SetVolume(double dVolume);
  double Volume();

  static bool IsAllowedToOverwriteKeyBinding(const QString& sRole);

signals:
  void ContentFolderChanged();
  void FontChanged();
  void FullscreenChanged();
  void KeyBindingsChanged();
  void MutedChanged();
  void ResolutionChanged();
  void StyleChanged();
  void VolumeChanged();

private:
  void GenerateSettingsIfNotExists();

  mutable QMutex             m_settingsMutex;
  std::shared_ptr<QSettings> m_spSettings;

  const std::map<QString, QKeySequence> c_sDefaultKeyBindings;
};

#endif // CSETTINGS_H
