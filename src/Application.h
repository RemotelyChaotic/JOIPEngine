#ifndef CAPPLICATION_H
#define CAPPLICATION_H

#include "Settings.h"
#include <QApplication>
#include <map>
#include <memory>

class CThreadedSystem;

class CApplication : public QApplication
{
  Q_OBJECT

public:
  explicit CApplication(int argc, char *argv[]);
  ~CApplication();

  static CApplication* Instance();

  void Initialize();

  std::shared_ptr<CSettings> Settings() { return m_spSettings; }
  template<typename T> std::weak_ptr<T> System();

private:
  std::map<qint32, std::shared_ptr<CThreadedSystem>> m_spSystemsMap;
  std::shared_ptr<CSettings>                         m_spSettings;
  bool                                               m_bInitialized;
};

#endif // CAPPLICATION_H
