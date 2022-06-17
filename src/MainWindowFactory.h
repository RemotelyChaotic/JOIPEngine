#ifndef CMAINWINDOWFACTORY_H
#define CMAINWINDOWFACTORY_H

#include <QMainWindow>
#include <memory>

class CMainWindowBase : public QMainWindow
{
  Q_OBJECT

public:
  CMainWindowBase(QWidget* pParent = nullptr) :
    QMainWindow(pParent)
  {}
  ~CMainWindowBase() override = default;

  virtual void ConnectSlots() = 0;
  virtual void Initialize() = 0;
};

//----------------------------------------------------------------------------------------
//
class CMainWindowFactory
{
public:
  static CMainWindowFactory& Instance();

  std::unique_ptr<CMainWindowBase> CreateMainWindow(QWidget* pParent = nullptr);

private:
  CMainWindowFactory();
};

#endif // CMAINWINDOWFACTORY_H
