#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>

namespace Ui {
  class CMainWindow;
}

class CMainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit CMainWindow(QWidget *parent = nullptr);
  ~CMainWindow();

  void Initialize();

private:
  std::unique_ptr<Ui::CMainWindow> m_spUi;
  bool                             m_bInitialized;
};

#endif // MAINWINDOW_H
