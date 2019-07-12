#ifndef MAINSCREEN_H
#define MAINSCREEN_H

#include <QWidget>
#include <memory>

namespace Ui {
  class CMainScreen;
}

class CMainScreen : public QWidget
{
  Q_OBJECT

public:
  explicit CMainScreen(QWidget *parent = nullptr);
  ~CMainScreen();

  void Initialize();

private:
  std::unique_ptr<Ui::CMainScreen> m_spUi;
  bool                             m_bInitialized;
};

#endif // MAINSCREEN_H
