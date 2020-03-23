#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Enums.h"
#include <QMainWindow>
#include <memory>

class CHelpButtonOverlay;
class CHelpOverlay;
class CSettings;
class CWindowContext;
namespace Ui {
  class CMainWindow;
}

class CMainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit CMainWindow(QWidget* pParent = nullptr);
  ~CMainWindow();

  void Initialize();

protected slots:
  void SlotChangeAppState(EAppState newState);
  void SlotFullscreenChanged();
  void SlotHelpButtonClicked();
  void SlotResolutionChanged();
  void SlotSetHelpButtonVisible(bool bVisible);

private:
  void ConnectSlots();

  std::unique_ptr<Ui::CMainWindow>    m_spUi;
  std::unique_ptr<CHelpButtonOverlay> m_spHelpButtonOverlay;
  std::unique_ptr<CHelpOverlay>       m_spHelpOverlay;
  std::shared_ptr<CWindowContext>     m_spWindowContext;
  std::shared_ptr<CSettings>          m_spSettings;
  bool                                m_bInitialized;
};

#endif // MAINWINDOW_H
