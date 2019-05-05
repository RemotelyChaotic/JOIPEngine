#ifndef CSETTINGS_H
#define CSETTINGS_H

#include <QObject>
#include <QString>

class CSettings : public QObject {
  Q_OBJECT

public:
  static const QString c_sSettingContentFolder;

  static const QString c_sOrganisation;
  static const QString c_sApplicationName;
};

#endif // CSETTINGS_H
