#ifndef CAPPLICATION_H
#define CAPPLICATION_H

#include "Settings.h"
#include <QGuiApplication>
#include <memory>

class CApplication : public QGuiApplication
{
  Q_OBJECT

public:
  explicit CApplication(int argc, char *argv[]);
  ~CApplication();

  static CApplication* Instance();

  void Initialize();

  std::shared_ptr<CSettings> Settings() { return m_spSettings; }

private:
  std::shared_ptr<CSettings> m_spSettings;
  bool                       m_bInitialized;
};

#endif // CAPPLICATION_H