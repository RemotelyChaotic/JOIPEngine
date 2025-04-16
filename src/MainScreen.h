#ifndef MAINSCREEN_H
#define MAINSCREEN_H

#include "IAppStateScreen.h"
#include <QPointer>
#include <QWidget>
#include <memory>

class CDownloadButtonOverlay;
class CHelpButtonOverlay;
namespace Ui {
  class CMainScreen;
}

class CMainScreen : public QWidget, public IAppStateScreen
{
  Q_OBJECT
  Q_INTERFACES(IAppStateScreen)

public:
  explicit CMainScreen(const std::shared_ptr<CWindowContext>& spWindowContext,
                       QWidget* pParent = nullptr);
  ~CMainScreen() override;

  bool CloseApplication() override { return false; }
  void Initialize() override;
  void Load() override;
  void Unload() override;

signals:
  void UnloadFinished() override;

protected slots:
  void on_pSceneSelectButton_clicked();
  void on_pEdiorButton_clicked();
  void on_pSettingsButton_clicked();
  void on_pCreditsButton_clicked();
  void on_pQuitButton_clicked();
  void SlotStyleLoaded();

protected:
  void resizeEvent(QResizeEvent* pEvt) override;

private:
  std::unique_ptr<Ui::CMainScreen> m_spUi;
  QPointer<CHelpButtonOverlay>     m_pHelpButtonOverlay;
  QPointer<CDownloadButtonOverlay> m_pDownloadButtonOverlay;
};

#endif // MAINSCREEN_H
