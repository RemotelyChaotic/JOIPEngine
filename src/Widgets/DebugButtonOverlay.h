#ifndef DEBUGBUTTONOVERLAY_H
#define DEBUGBUTTONOVERLAY_H

#include "OverlayButton.h"
#include <QPointer>

class CDebugButtonOverlay : public COverlayButton
{
  Q_OBJECT

public:
  explicit CDebugButtonOverlay(QWidget* pParent = nullptr);
  ~CDebugButtonOverlay() override;

public slots:
  void Hide() override;
  void Resize() override;
  void Show() override;
};

#endif // DEBUGBUTTONOVERLAY_H
