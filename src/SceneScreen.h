#ifndef PROJECTSELECTIONSCREEN_H
#define PROJECTSELECTIONSCREEN_H

#include "IAppStateScreen.h"
#include <QWidget>
#include <memory>

class CDatabaseManager;
typedef std::shared_ptr<struct SProject> tspProject;
namespace Ui {
  class CSceneScreen;
}

class CSceneScreen : public QWidget, public IAppStateScreen
{
  Q_OBJECT
  Q_INTERFACES(IAppStateScreen)

public:
  explicit CSceneScreen(const std::shared_ptr<CWindowContext>& spWindowContext,
                        QWidget* pParent = nullptr);
  ~CSceneScreen() override;

  bool CloseApplication() override;
  void Initialize() override;
  void Load() override;
  void Unload() override;

signals:
  void UnloadFinished() override;

protected slots:
  void on_pOpenExistingProjectButton_clicked();
  void on_pOpenExistingProjectAtSceneButton_clicked();
  void on_pCancelButton_clicked();
  void on_pProjectCardSelectionWidget_SingalSelected(qint32 iId);
  void SlotCardsUnloadFinished();
  void SlotExitClicked();
  void SlotSceneUnloadFinished();

private:
  tspProject GetDataAndProject(qint32 iId, QStringList& vsScenes);
  qint32 GetProjectDataFromSelection(QStringList& vsScenes);
  void OpenProject(qint32 iId, const QString& sScene);

  std::unique_ptr<Ui::CSceneScreen>        m_spUi;
  std::weak_ptr<CDatabaseManager>          m_wpDbManager;
};

#endif // PROJECTSELECTIONSCREEN_H
