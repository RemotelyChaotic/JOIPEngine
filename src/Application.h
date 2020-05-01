#ifndef CAPPLICATION_H
#define CAPPLICATION_H

#include "Settings.h"
#include "Systems/DatabaseManager.h"
#include <QApplication>
#include <map>
#include <memory>

class CHelpFactory;
class COverlayManager;
class CScriptRunner;
class CThreadedSystem;
class CUISoundEmitter;

class CApplication : public QApplication
{
  Q_OBJECT

public:
  explicit CApplication(int& argc, char *argv[]);
  ~CApplication();

  static CApplication* Instance();

  void Initialize();

  std::shared_ptr<CSettings> Settings() { return m_spSettings; }
  template<typename T> std::weak_ptr<T> System();

signals:
  void StyleLoaded();

private slots:
  void MarkStyleDirty();
  void LoadStyle();

private:
  void RegisterQmlTypes();

  std::map<qint32, std::shared_ptr<CThreadedSystem>> m_spSystemsMap;
  std::unique_ptr<CUISoundEmitter>                   m_spSoundEmitter;
  std::shared_ptr<CHelpFactory>                      m_spHelpFactory;
  std::shared_ptr<COverlayManager>                   m_spOverlayManager;
  std::shared_ptr<CSettings>                         m_spSettings;
  bool                                               m_bStyleDirty;
  bool                                               m_bInitialized;
};

template<> std::weak_ptr<CDatabaseManager> CApplication::System<CDatabaseManager>();
template<> std::weak_ptr<CHelpFactory> CApplication::System<CHelpFactory>();
template<> std::weak_ptr<COverlayManager> CApplication::System<COverlayManager>();
template<> std::weak_ptr<CScriptRunner> CApplication::System<CScriptRunner>();

#endif // CAPPLICATION_H
