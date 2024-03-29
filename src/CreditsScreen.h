#ifndef CREDITSSCREEN_H
#define CREDITSSCREEN_H

#include "IAppStateScreen.h"
#include <QWidget>
#include <memory>

namespace Ui {
  class CCreditsScreen;
}

class CCreditsScreen : public QWidget, public IAppStateScreen
{
  Q_OBJECT
  Q_INTERFACES(IAppStateScreen)

public:
  explicit CCreditsScreen(const std::shared_ptr<CWindowContext>& spWindowContext,
                          QWidget* pParent = nullptr);
  ~CCreditsScreen() override;

  bool CloseApplication() override { return false; }
  void Initialize() override;
  void Load() override;
  void Unload() override;

signals:
  void UnloadFinished() override;

protected slots:
  void on_pBackButton_clicked();
  void on_pAboutQtButton_clicked();
  void SlotStyleLoaded();

private:
  std::unique_ptr<Ui::CCreditsScreen> m_spUi;
};

#endif // CREDITSSCREEN_H
