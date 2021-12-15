#ifndef SCENEMAINSCREEN_H
#define SCENEMAINSCREEN_H

#include <QPointer>
#include <QQuickWidget>
#include <QWidget>
#include <memory>

class CBackgroundWidget;
class CDatabaseManager;
class CProjectEventCallbackRegistry;
class CProjectScriptWrapper;
class CProjectRunner;
class CScriptRunner;
class CSettings;
class CThreadedSystem;
namespace Ui {
  class CSceneMainScreen;
}
struct SProject;
typedef std::shared_ptr<SProject> tspProject;

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

  void Initialize();
  void LoadProject(qint32 iId, const QString sStartScene = QString());
  std::weak_ptr<CProjectEventCallbackRegistry> EventCallbackRegistry();
  std::weak_ptr<CProjectRunner> ProjectRunner();
  std::weak_ptr<CScriptRunner> ScriptRunner();
  void UnloadProject();
  void SetDebugging(bool bDebugging);

public slots:
  void SlotFinish();
  void SlotQuit();

signals:
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
  void SlotNextSkript();
  void SlotResizeDone();
  void SlotSceneSelectReturnValue(int iIndex);
  void SlotScriptRunFinished(bool bOk, const QString& sRetVal);
  void SlotStartLoadingSkript();
  void SlotUnloadFinished();

private:
  void ConnectAllSignals();
  void DisconnectAllSignals();
  void Finish();
  void InitQmlMain();
  void LoadQml();
  void NextSkript();
  void UnloadRunner();
  void UnloadQml();

private:
  std::unique_ptr<Ui::CSceneMainScreen>                       m_spUi;
  std::shared_ptr<CProjectEventCallbackRegistry>              m_spEventCallbackRegistry;
  std::shared_ptr<CProjectRunner>                             m_spProjectRunner;
  std::shared_ptr<CThreadedSystem>                            m_spScriptRunnerSystem;
  std::shared_ptr<CScriptRunner>                              m_spScriptRunner;
  std::shared_ptr<CSettings>                                  m_spSettings;
  tspProject                                                  m_spCurrentProject;
  QPointer<CProjectScriptWrapper>                             m_pCurrentProjectWrapper;
  std::weak_ptr<CDatabaseManager>                             m_wpDbManager;
  qint32                                                      m_lastScriptExecutionStatus;
  bool                                                        m_bInitialized;
  bool                                                        m_bShuttingDown;
  bool                                                        m_bErrorState;
  bool                                                        m_bBeingDebugged;
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
