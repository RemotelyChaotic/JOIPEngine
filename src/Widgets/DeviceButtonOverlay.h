#ifndef CDEVICEBUTTONOVERLAY_H
#define CDEVICEBUTTONOVERLAY_H

#include "OverlayButton.h"
#include <QPointer>

#include <memory>

class CDeviceManager;
class CProgressBar;
class CTitleLabel;

class CDeviceCounterOverlay : public COverlayBase
{
  Q_OBJECT

public:
  explicit CDeviceCounterOverlay(QWidget* pParent = nullptr);
  ~CDeviceCounterOverlay() override;

  void Initialize();

  QPointer<CTitleLabel> Counter();

public slots:
  void Climb() override;
  void Resize() override;

private slots:
  void SlotConnected();
  void SlotDeviceCountChanged();
  void SlotDisconnected();

private:
  std::weak_ptr<CDeviceManager> m_wpDeviceManager;
  QPointer<CTitleLabel>         m_pDlCounter;
};

//----------------------------------------------------------------------------------------
//
class CDeviceButtonOverlay : public COverlayButton
{
  Q_OBJECT

public:
  CDeviceButtonOverlay(QWidget* pParent = nullptr);
  ~CDeviceButtonOverlay() override;

public slots:
  void Hide() override;
  void Resize() override;
  void Show() override;

protected:
  void contextMenuEvent(QContextMenuEvent* pEvent) override;

private slots:
  void SlotStartScanning();
  void SlotStopScanning();

private:
  void OpenContextMenuAt(const QPoint& localPoint, const QPoint& createPoint);

  std::weak_ptr<CDeviceManager>   m_wpDeviceManager;
  QPointer<CProgressBar>            m_pProgressBar;
  QPointer<CDeviceCounterOverlay> m_pCounterOverlay;
};

#endif // CDEVICEBUTTONOVERLAY_H
