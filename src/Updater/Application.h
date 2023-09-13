#ifndef CAPPLICATION_H
#define CAPPLICATION_H

#include "SettingsData.h"

#include <QApplication>
#include <QObject>

#include <memory>

class CApplication : public QApplication
{
  Q_OBJECT
public:
  CApplication(int &argc, char **argv);
  ~CApplication() override;

  static CApplication* Instance();

  void SetSettings(SSettingsData* pSettings);
  SSettingsData* Settings() const;

signals:
  void StyleLoaded(); // unused, just mock

private:
  SSettingsData* m_pSettings = nullptr;
};

#endif // CAPPLICATION_H
