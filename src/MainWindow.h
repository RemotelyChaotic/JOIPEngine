#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Enums.h"
#include <QMainWindow>
#include <QPointer>
#include <memory>

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
  void SlotFullscreenChanged();
  void SlotHelpButtonClicked();
  void SlotResolutionChanged();
  void SlotSetHelpButtonVisible(bool bVisible);

protected:
  void resizeEvent(QResizeEvent* pEvt) override;

private:
  void ConnectSlots();

  std::unique_ptr<Ui::CMainWindow>    m_spUi;
  std::unique_ptr<CHelpButtonOverlay> m_spHelpButtonOverlay;
  std::unique_ptr<CHelpOverlay>       m_spHelpOverlay;
  std::shared_ptr<CWindowContext>     m_spWindowContext;
  std::shared_ptr<CSettings>          m_spSettings;
  QPointer<CBackgroundWidget>         m_pBackground;
  bool                                m_bInitialized;
  EAppState                           m_nextState;
};

#endif // MAINWINDOW_H
