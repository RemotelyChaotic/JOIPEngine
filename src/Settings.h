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
  Q_PROPERTY(QStringList keyBindings READ KeyBindings)
  Q_PROPERTY(bool muted READ Muted WRITE SetMuted NOTIFY mutedChanged)
  Q_PROPERTY(bool offline READ Offline WRITE SetOffline NOTIFY offlineChanged)
  Q_PROPERTY(QSize resolution READ Resolution WRITE SetResolution NOTIFY resolutionChanged)
  Q_PROPERTY(QString style READ Style WRITE SetStyle NOTIFY styleChanged)
  Q_PROPERTY(qint32 version READ Version)
  Q_PROPERTY(double volume READ Volume WRITE SetVolume NOTIFY volumeChanged)

public:
  static const QString c_sSettingContentFolder;
  static const QString c_sSettingFont;
  static const QString c_sSettingFullscreen;
  static const QString c_sSettingKeyBindings;
  static const QString c_sSettingMuted;
  static const QString c_sSettingOffline;
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
  Q_INVOKABLE QKeySequence keyBinding(const QString& sRole);
  Q_INVOKABLE void setKeyBinding(const QKeySequence& sKeySequence, const QString& sRole);
  void SetMuted(bool bValue);
  bool Muted();
  void SetOffline(bool bValue);
  bool Offline();
  void SetResolution(const QSize& size);
  QSize Resolution();
  void SetStyle(const QString& sStyle);
  QString Style();
  Q_INVOKABLE QUrl styleFolder();
  Q_INVOKABLE QUrl styleFolderQml();
  qint32 Version();
  void SetVolume(double dVolume);
  double Volume();

  static bool IsAllowedToOverwriteKeyBinding(const QString& sRole);

signals:
  void contentFolderChanged();
  void fontChanged();
  void fullscreenChanged();
  void keyBindingsChanged();
  void mutedChanged();
  void offlineChanged();
  void resolutionChanged();
  void styleChanged();
  void volumeChanged();

private:
  void GenerateSettingsIfNotExists();

  mutable QMutex             m_settingsMutex;
  std::shared_ptr<QSettings> m_spSettings;

  const std::map<QString, QKeySequence> c_sDefaultKeyBindings;
};

Q_DECLARE_METATYPE(CSettings*)

#endif // CSETTINGS_H
