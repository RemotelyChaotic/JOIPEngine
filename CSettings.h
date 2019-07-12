#ifndef CSETTINGS_H
#define CSETTINGS_H

#include <QObject>
#include <QString>
#include <memory>

class QSettings;

class CSettings : public QObject {

  Q_OBJECT
  Q_PROPERTY(QString contentFolder READ ContentFolder WRITE SetContentFolder NOTIFY ContentFolderChanged)

public:
  static const QString c_sSettingContentFolder;

  static const QString c_sOrganisation;
  static const QString c_sApplicationName;

  explicit CSettings(QObject* pParent = nullptr);
  ~CSettings() override;

  void SetContentFolder(const QString& sPath);
  QString ContentFolder();

signals:
  void ContentFolderChanged();

private:
  void GenerateSettingsIfNotExists();

  std::shared_ptr<QSettings> m_spSettings;
};

#endif // CSETTINGS_H
