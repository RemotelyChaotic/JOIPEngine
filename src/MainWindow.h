#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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

class CMainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit CMainWindow(QWidget* pParent = nullptr);
  ~CMainWindow();

  void Initialize();

protected slots:
  void SlotChangeAppState(EAppState newState);
  void SlotCurrentAppStateUnloadFinished();
  void SlotDownloadButtonClicked();
  void SlotFullscreenChanged();
  void SlotHelpButtonClicked();
  void SlotResolutionChanged();
  void SlotSetDownloadButtonVisible(bool bVisible);
  void SlotSetHelpButtonVisible(bool bVisible);

protected:
  void closeEvent(QCloseEvent* pEvent) override;
  void resizeEvent(QResizeEvent* pEvt) override;

private slots:
  void OldSettingsDetected();

private:
  void ConnectSlots();

  std::unique_ptr<Ui::CMainWindow>        m_spUi;
  std::unique_ptr<CHelpButtonOverlay>     m_spHelpButtonOverlay;
  std::unique_ptr<CHelpOverlay>           m_spHelpOverlay;
  std::unique_ptr<CDownloadButtonOverlay> m_spDownloadButtonOverlay;
  std::shared_ptr<CWindowContext>         m_spWindowContext;
  std::shared_ptr<CSettings>              m_spSettings;
  QPointer<CBackgroundWidget>             m_pBackground;
  bool                                    m_bInitialized;
  EAppState                               m_nextState;
};

#endif // MAINWINDOW_H
