#ifndef CLINUXMAINWINDOW_H
#define CLINUXMAINWINDOW_H

#include "MainWindow.h"

class CLinuxMainWindow : public CMainWindow
{
  Q_OBJECT

public:
  CLinuxMainWindow(QWidget* pParent = nullptr);
  ~CLinuxMainWindow() override;

  void OnShow() override;

protected slots:
  void SlotResolutionChanged() override;
  void SlotWindowModeChanged() override;

protected:
  void ConnectSlotsImpl() override;

private slots:
  void OldSettingsDetected() override;

private:
};

#endif // CLINUXMAINWINDOW_H
