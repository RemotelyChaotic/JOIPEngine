#ifndef DOWNLOADSCREEN_H
#define DOWNLOADSCREEN_H

#include "IAppStateScreen.h"
#include <QWidget>
#include <memory>

class CDatabaseManager;
namespace Ui {
class CDownloadScreen;
}

class CDownloadScreen : public QWidget, public IAppStateScreen
{
  Q_OBJECT
  Q_INTERFACES(IAppStateScreen)

public:
  explicit CDownloadScreen(const std::shared_ptr<CWindowContext>& spWindowContext,
                           QWidget *parent = nullptr);
  ~CDownloadScreen();

  void Initialize() override;
  void Load() override;
  void Unload() override;

signals:
  void UnloadFinished() override;

protected slots:
  void on_pDownloadButton_clicked();
  void on_pCancelButton_clicked();
  void SlotCardsUnloadFinished();
  void SlotExitClicked();

private:
  std::unique_ptr<Ui::CDownloadScreen> m_spUi;
  std::weak_ptr<CDatabaseManager>      m_wpDbManager;
};

#endif // DOWNLOADSCREEN_H
