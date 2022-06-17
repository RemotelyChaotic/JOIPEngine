#ifndef CWINDOWSMAINWINDOW_H
#define CWINDOWSMAINWINDOW_H

#include "MainWindow.h"

class CWindowsMainWindow : public CMainWindow
{
  Q_OBJECT

public:
  CWindowsMainWindow(QWidget* pParent = nullptr);
  ~CWindowsMainWindow() override;

protected slots:
  void SlotResolutionChanged() override;
  void SlotWindowModeChanged() override;

protected:
  void ConnectSlotsImpl() override;

private slots:
  void OldSettingsDetected() override;
};

#endif // CWINDOWSMAINWINDOW_H
