#ifndef CANDROIDMAINWINDOW_H
#define CANDROIDMAINWINDOW_H

#include "MainWindow.h"

#include <QColor>

class CAndroidNavigationBar;

class CAndroidMainWindow : public CMainWindow
{
  Q_OBJECT
  Q_PROPERTY(QColor navigationColor READ NavigationColor WRITE SetNavigationColor)

public:
  CAndroidMainWindow(QWidget* pParent = nullptr);
  ~CAndroidMainWindow() override;

  QColor NavigationColor() const;
  void SetNavigationColor(const QColor& color);

protected slots:
  void SlotResolutionChanged() override;
  void SlotWindowModeChanged() override;

protected:
  void ConnectSlotsImpl() override;

private slots:
  void OldSettingsDetected() override;

private:
  QPointer<CAndroidNavigationBar> m_pNavigationBar;
};

#endif // CANDROIDMAINWINDOW_H
