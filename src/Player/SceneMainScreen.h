#ifndef SCENEMAINSCREEN_H
#define SCENEMAINSCREEN_H

#include <QPointer>
#include <QQuickWidget>
#include <QWidget>
#include <memory>
#include <variant>

class CBackgroundWidget;
class CDatabaseManager;
class CPlayerConsoleError;
class CProjectEventCallbackRegistry;
class CProjectScriptWrapper;
class CProjectRunner;
class CScriptRunner;
class CSettings;
class CThreadedSystem;
class CWindowContext;
namespace Ui {
  class CSceneMainScreen;
}
struct SProject;
typedef std::shared_ptr<SProject> tspProject;
struct SScene;
typedef std::shared_ptr<SScene> tspScene;

namespace player {
  extern const char* c_sMainPlayerProperty;
}

//----------------------------------------------------------------------------------------
//
class CSceneMainScreen : public QWidget
{
  Q_OBJECT

public:
  explicit CSceneMainScreen(QWidget* pParent = nullptr);
  ~CSceneMainScreen() override;

  using tSceneToLoad = std::variant<QString, tspScene>;

  bool CloseApplication();
  void Initialize(const std::shared_ptr<CWindowContext>& spWindowContext, bool bDebug);
  void LoadProject(qint32 iId, const tSceneToLoad& sStartScene = QString());
  std::weak_ptr<CProjectEventCallbackRegistry> EventCallbackRegistry();
  std::weak_ptr<CProjectRunner> ProjectRunner();
  std::weak_ptr<CScriptRunner> ScriptRunner();
  void UnloadProject();
  void SetDebugging(bool bDebugging);

public slots:
  void SlotFinish();
  void SlotQuit();

signals:
  void SignalExecutionError(QString sException, qint32 iLine, QString sStack);
  void SignalExitClicked();
  void SignalUnloadFinished();

protected:
  void resizeEvent(QResizeEvent* pEvent) override;

private slots:
  void on_pQmlWidget_statusChanged(QQuickWidget::Status);
  void on_pQmlWidget_sceneGraphError(QQuickWindow::SceneGraphError error, const QString &message);
  void SlotApplicationStateChanged(Qt::ApplicationState state);
  void SlotError(QString sError, QtMsgType type);
  void SlotExecutionError(QString sException, qint32 iLine, QString sStack);
  void SlotNextSkript(bool bMightBeRegex);
  void SlotQmlEngineWarning(const QList<QQmlError>& vWarnings);
  void SlotResizeDone();
  void SlotSceneSelectReturnValue(int iIndex);
  void SlotSceneLoaded(const QString& sScene);
  void SlotScriptRunFinished(bool bOk, const QString& sRetVal);
  void SlotStartLoadingSkript();
  void SlotUnloadFinished();

private:
  void ConnectAllSignals();
  void DisconnectAllSignals();
  void Finish();
  void InitQmlMain();
  void LoadQml();
  void NextSkript(bool bMightBeRegex);
  void UnloadRunner();
  void UnloadQml();

private:
  std::unique_ptr<Ui::CSceneMainScreen>                       m_spUi;
  std::shared_ptr<CWindowContext>                             m_spWindowContext;
  std::shared_ptr<CProjectEventCallbackRegistry>              m_spEventCallbackRegistry;
  std::shared_ptr<CProjectRunner>                             m_spProjectRunner;
  std::shared_ptr<CThreadedSystem>                            m_spScriptRunnerSystem;
  std::shared_ptr<CScriptRunner>                              m_spScriptRunner;
  std::shared_ptr<CSettings>                                  m_spSettings;
  tspProject                                                  m_spCurrentProject;
  QPointer<CProjectScriptWrapper>                             m_pCurrentProjectWrapper;
  QPointer<CPlayerConsoleError>                               m_pErrorConsole;
  std::weak_ptr<CDatabaseManager>                             m_wpDbManager;
  QStringList                                                 m_vsBaseImportPathList;
  qint32                                                      m_lastScriptExecutionStatus;
  bool                                                        m_bInitialized;
  bool                                                        m_bShuttingDown;
  bool                                                        m_bErrorState;
  bool                                                        m_bBeingDebugged;
  bool                                                        m_bCloseRequested;
  bool                                                        m_bCanLoadNewScene;
};

//----------------------------------------------------------------------------------------
//
class CSceneMainScreenWrapper : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CSceneMainScreenWrapper)

public:
  CSceneMainScreenWrapper(QObject* pParent, QPointer<CSceneMainScreen> pPlayer);
  ~CSceneMainScreenWrapper() override;

public:
  Q_INVOKABLE void initObject(QJSValue wrapper);

private:
  QPointer<CSceneMainScreen>                    m_pPlayer;
};

#endif // SCENEMAINSCREEN_H
