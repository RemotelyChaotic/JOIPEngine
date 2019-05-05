#ifndef CAPPLICATION_H
#define CAPPLICATION_H

#include <QGuiApplication>
#include <memory>

class QSettings;

class CApplication : public QGuiApplication
{
  Q_OBJECT

public:
  explicit CApplication(int argc, char *argv[]);
  ~CApplication();

  void Initialize();

  std::shared_ptr<QSettings> Settings() { return m_spSettings; }

private:
  void GenerateSettingsIfNotExists();

  std::shared_ptr<QSettings> m_spSettings;
  bool                       m_bInitialized;
};

#endif // CAPPLICATION_H
