#ifndef MAINSCREEN_H
#define MAINSCREEN_H

#include "IAppStateScreen.h"
#include <QWidget>
#include <memory>

namespace Ui {
  class CMainScreen;
}

class CMainScreen : public QWidget, public IAppStateScreen
{
  Q_OBJECT

public:
  explicit CMainScreen(const std::shared_ptr<CWindowContext>& spWindowContext,
                       QWidget* pParent = nullptr);
  ~CMainScreen() override;

  void Initialize() override;
  void Load() override;
  void Unload() override;

protected slots:
  void on_pSceneSelectButton_clicked();
  void on_pEdiorButton_clicked();
  void on_pSettingsButton_clicked();
  void on_pCreditsButton_clicked();
  void on_pQuitButton_clicked();
  void SlotStyleLoaded();

private:
  std::unique_ptr<Ui::CMainScreen> m_spUi;
};

#endif // MAINSCREEN_H
