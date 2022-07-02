#ifndef CAPPLICATION_H
#define CAPPLICATION_H

#include "Settings.h"
#include "Systems/DatabaseManager.h"
#include <QApplication>
#include <QFileSystemWatcher>
#include <map>
#include <memory>

class CBackActionHandler;
class CHelpFactory;
class CNotificationSender;
class COverlayManager;
class CProjectDownloader;
class CThreadedSystem;
class CUISoundEmitter;
class QQmlEngine;

class CQmlApplicationQtNamespaceWrapper : public QObject
{
  Q_OBJECT

public:
  explicit CQmlApplicationQtNamespaceWrapper(QObject* eParent = nullptr);
  ~CQmlApplicationQtNamespaceWrapper();

public slots:
  QString decodeHTML(const QString& sString);
  bool mightBeRichtext(const QString& sString);
};

//----------------------------------------------------------------------------------------
//
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
  void SendNotification(const QString& sTitle, const QString& sMsg);

signals:
  void StyleLoaded();

private slots:
  void MarkStyleDirty();
  void LoadStyle();
  void StyleHotloadChanged();

private:
  void RegisterQmlTypes();

  std::map<qint32, std::shared_ptr<CThreadedSystem>> m_spSystemsMap;
  std::unique_ptr<CUISoundEmitter>                   m_spSoundEmitter;
  std::shared_ptr<CHelpFactory>                      m_spHelpFactory;
  std::shared_ptr<COverlayManager>                   m_spOverlayManager;
  std::shared_ptr<CSettings>                         m_spSettings;
  std::shared_ptr<CNotificationSender>               m_spNotifier;
  std::shared_ptr<CBackActionHandler>                m_spBackActionHandler;
  QFileSystemWatcher                                 m_styleWatcher;
  bool                                               m_bStyleDirty;
  bool                                               m_bInitialized;
};

template<> std::weak_ptr<CDatabaseManager> CApplication::System<CDatabaseManager>();
template<> std::weak_ptr<CHelpFactory> CApplication::System<CHelpFactory>();
template<> std::weak_ptr<COverlayManager> CApplication::System<COverlayManager>();
template<> std::weak_ptr<CProjectDownloader> CApplication::System<CProjectDownloader>();
template<> std::weak_ptr<CBackActionHandler> CApplication::System<CBackActionHandler>();

#endif // CAPPLICATION_H
