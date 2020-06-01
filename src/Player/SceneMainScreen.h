#ifndef SCENEMAINSCREEN_H
#define SCENEMAINSCREEN_H

#include <QPointer>
#include <QQuickWidget>
#include <QWidget>
#include <memory>

class CBackgroundWidget;
class CDatabaseManager;
class CProject;
class CProjectRunner;
class CScriptRunner;
class CSettings;
namespace Ui {
  class CSceneMainScreen;
}
struct SProject;
typedef std::shared_ptr<SProject> tspProject;

class CSceneMainScreen : public QWidget
{
  Q_OBJECT

public:
  explicit CSceneMainScreen(QWidget* pParent = nullptr);
  ~CSceneMainScreen() override;

  void Initialize();
  void LoadProject(qint32 iId, const QString sStartScene = QString());
  void UnloadProject();

public slots:
  void SlotQuit();

signals:
  void SignalExitClicked();

protected:
  void resizeEvent(QResizeEvent* pEvent) override;

private slots:
  void on_pQmlWidget_statusChanged(QQuickWidget::Status);
  void on_pQmlWidget_sceneGraphError(QQuickWindow::SceneGraphError error, const QString &message);
  void SlotError(QString sError, QtMsgType type);
  void SlotExecutionError(QString sException, qint32 iLine, QString sStack);
  void SlotNextSkript();
  void SlotResizeDone();
  void SlotSceneSelectReturnValue(int iIndex);
  void SlotScriptRunFinished(bool bOk, const QString& sRetVal);
  void SlotStartLoadingSkript();

private:
  void ConnectAllSignals();
  void DisconnectAllSignals();
  void InitQmlMain();
  void NextSkript();

private:
  std::unique_ptr<Ui::CSceneMainScreen>                       m_spUi;
  std::unique_ptr<CProjectRunner>                             m_spProjectRunner;
  std::shared_ptr<CSettings>                                  m_spSettings;
  tspProject                                                  m_spCurrentProject;
  QPointer<CProject>                                          m_pCurrentProjectWrapper;
  std::weak_ptr<CDatabaseManager>                             m_wpDbManager;
  std::weak_ptr<CScriptRunner>                                m_wpScriptRunner;
  bool                                                        m_bInitialized;
};

#endif // SCENEMAINSCREEN_H
