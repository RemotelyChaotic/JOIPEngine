#ifndef CSETTINGS_H
#define CSETTINGS_H

#include <QMutex>
#include <QObject>
#include <QSize>
#include <QString>
#include <memory>

class QSettings;

class CSettings : public QObject {

  Q_OBJECT
  Q_PROPERTY(QString contentFolder READ ContentFolder WRITE SetContentFolder NOTIFY ContentFolderChanged)
  Q_PROPERTY(QSize resolution READ Resolution WRITE SetResolution NOTIFY ResolutionChanged)

public:
  static const QString c_sSettingResolution;
  static const QString c_sSettingContentFolder;

  static const QString c_sOrganisation;
  static const QString c_sApplicationName;

  explicit CSettings(QObject* pParent = nullptr);
  ~CSettings() override;

  void SetContentFolder(const QString& sPath);
  QString ContentFolder();
  void SetResolution(const QSize& size);
  QSize Resolution();

signals:
  void ContentFolderChanged();
  void ResolutionChanged();

private:
  void GenerateSettingsIfNotExists();

  mutable QMutex             m_settingsMutex;
  std::shared_ptr<QSettings> m_spSettings;
};

#endif // CSETTINGS_H
