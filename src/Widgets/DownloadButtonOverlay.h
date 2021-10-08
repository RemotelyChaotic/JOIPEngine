#ifndef CDOWNLOADBUTTONOVERLAY_H
#define CDOWNLOADBUTTONOVERLAY_H

#include "OverlayButton.h"

class CDownloadButtonOverlay : public COverlayButton
{
  Q_OBJECT

public:
  explicit CDownloadButtonOverlay(QWidget* pParent = nullptr);
  ~CDownloadButtonOverlay() override;

  void Resize() override;
};

#endif // CDOWNLOADBUTTONOVERLAY_H
