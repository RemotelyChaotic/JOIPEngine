#ifndef SCENEMAINSCREEN_H
#define SCENEMAINSCREEN_H

#include <QPointer>
#include <QWidget>
#include <memory>

class CBackgroundWidget;
class CDatabaseManager;
class CProjectRunner;
class CScriptRunner;
class CSettings;
namespace Ui {
  class CSceneMainScreen;
}
class QGraphicsScene;
class QGraphicsView;
struct SProject;
struct SResource;
typedef std::shared_ptr<SProject> tspProject;
typedef std::shared_ptr<SResource> tspResource;

class CSceneMainScreen : public QWidget
{
  Q_OBJECT

public:
  explicit CSceneMainScreen(QWidget* pParent = nullptr);
  ~CSceneMainScreen() override;

  void Initialize();
  void LoadProject(qint32 iId);
  void UnloadProject();

signals:
  void SignalExitClicked();

protected:
  void resizeEvent(QResizeEvent* pEvent) override;

private slots:
  void SlotError(QString sError, QtMsgType type);
  void SlotNextSkript();
  void SlotPauseVideo();
  void SlotPauseSound();
  void SlotPlayMedia(tspResource spResource);
  void SlotQuit();
  void SlotSceneSelectReturnValue(qint32 iIndex);
  void SlotScriptRunFinished(bool bOk);
  void SlotShowMedia(tspResource spResource);
  void SlotStopVideo();
  void SlotStopSound();

private:
  void ConnectAllSignals();
  void DisconnectAllSignals();
  void NextSkript();

private:
  std::unique_ptr<Ui::CSceneMainScreen>                       m_spUi;
  std::unique_ptr<CProjectRunner>                             m_spProjectRunner;
  std::shared_ptr<CSettings>                                  m_spSettings;
  tspProject                                                  m_spCurrentProject;
  std::weak_ptr<CDatabaseManager>                             m_wpDbManager;
  std::weak_ptr<CScriptRunner>                                m_wpScriptRunner;
  QPointer<CBackgroundWidget>                                 m_pBackground;
  QPointer<QAction>                                           m_pActionSkip;
  QPointer<QAction>                                           m_pActionQuit;
  bool                                                        m_bInitialized;
};

#endif // SCENEMAINSCREEN_H
