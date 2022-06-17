#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "MainWindowFactory.h"
#include "Enums.h"
#include <QMainWindow>
#include <QPointer>
#include <memory>

class CDownloadButtonOverlay;
class CHelpButtonOverlay;
class CHelpOverlay;
class CBackgroundWidget;
class CSettings;
class CWindowContext;
namespace Ui {
  class CMainWindow;
}
class QResizeEvent;

class CMainWindow : public CMainWindowBase
{
  Q_OBJECT

public:
  explicit CMainWindow(QWidget* pParent = nullptr);
  ~CMainWindow() override;

  void ConnectSlots() override;
  void Initialize() override;

protected slots:
  void SlotChangeAppState(EAppState newState);
  void SlotCurrentAppStateUnloadFinished();
  void SlotDownloadButtonClicked();
  void SlotHelpButtonClicked();
  void SlotSetDownloadButtonVisible(bool bVisible);
  void SlotSetHelpButtonVisible(bool bVisible);

  virtual void SlotResolutionChanged() {}
  virtual void SlotWindowModeChanged() {}

protected:
  void closeEvent(QCloseEvent* pEvent) override;
  void resizeEvent(QResizeEvent* pEvt) override;

  virtual void ConnectSlotsImpl() {}

  std::unique_ptr<Ui::CMainWindow>        m_spUi;
  std::unique_ptr<CHelpButtonOverlay>     m_spHelpButtonOverlay;
  std::unique_ptr<CHelpOverlay>           m_spHelpOverlay;
  std::unique_ptr<CDownloadButtonOverlay> m_spDownloadButtonOverlay;
  std::shared_ptr<CWindowContext>         m_spWindowContext;
  std::shared_ptr<CSettings>              m_spSettings;
  QPointer<CBackgroundWidget>             m_pBackground;

private slots:
  virtual void OldSettingsDetected() {};

private:
  bool                                    m_bInitialized;
  EAppState                               m_nextState;
};

#endif // MAINWINDOW_H
