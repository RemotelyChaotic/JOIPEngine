#ifndef CWINDOWSMAINWINDOW_H
#define CWINDOWSMAINWINDOW_H

#include "MainWindow.h"

class QWinTaskbarButton;
class QWinTaskbarProgress;

class CWindowsMainWindow : public CMainWindow
{
  Q_OBJECT

public:
  CWindowsMainWindow(QWidget* pParent = nullptr);
  ~CWindowsMainWindow() override;

  void OnShow() override;

protected slots:
  void SlotResolutionChanged() override;
  void SlotWindowModeChanged() override;

protected:
  void ConnectSlotsImpl() override;

private slots:
  void OldSettingsDetected() override;
  void SlotChangeAppOverlay(const QString& sImage);
  void SlotDownloadFinished(qint32 iProjId);
  void SlotDownloadStarted(qint32 iProjId);
  void SlotProgressChanged(qint32 iProjId, qint32 iProgress);

private:
  QPointer<QWinTaskbarButton>   m_pTaskButton;
  QPointer<QWinTaskbarProgress> m_pTaskProgress;
};

#endif // CWINDOWSMAINWINDOW_H
