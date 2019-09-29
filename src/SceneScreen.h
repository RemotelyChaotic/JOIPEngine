#ifndef PROJECTSELECTIONSCREEN_H
#define PROJECTSELECTIONSCREEN_H

#include "IAppStateScreen.h"
#include <QWidget>
#include <memory>

class CDatabaseManager;
namespace Ui {
  class CSceneScreen;
}

class CSceneScreen : public QWidget, public IAppStateScreen
{
  Q_OBJECT

public:
  explicit CSceneScreen(const std::shared_ptr<CWindowContext>& spWindowContext,
                                 QWidget* pParent = nullptr);
  ~CSceneScreen() override;

  void Initialize() override;
  void Load() override;
  void Unload() override;

protected slots:
  void on_pOpenExistingProjectButton_clicked();
  void on_pCancelButton_clicked();
  void SlotExitClicked();

private:
  std::unique_ptr<Ui::CSceneScreen>        m_spUi;
  std::weak_ptr<CDatabaseManager>          m_wpDbManager;
};

#endif // PROJECTSELECTIONSCREEN_H
