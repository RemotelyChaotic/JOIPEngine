#ifndef DEVICESNIPPETOVERLAY_H
#define DEVICESNIPPETOVERLAY_H

#include "CodeSnippetOverlayBase.h"

#include <QWidget>

#include <memory>

namespace Ui {
  class CDeviceSnippetOverlay;
}

class CDeviceSnippetOverlay : public CCodeSnippetOverlayBase
{
  Q_OBJECT

public:
  explicit CDeviceSnippetOverlay(QWidget* pParent = nullptr);
  ~CDeviceSnippetOverlay();

public slots:
  void Resize() override;

protected slots:
  void on_pVibrateGroupBox_toggled(bool bChecked);
  void on_pSpeedVibrateSpinBox_valueChanged(double dValue);
  void on_pLinearGroupBox_toggled(bool bChecked);
  void on_pDurationSpinBox_valueChanged(double dValue);
  void on_pPositionSpinBox_valueChanged(double dValue);
  void on_pRotateGroupBox_toggled(bool bChecked);
  void on_pClockwiseCheckBox_toggled(bool bChecked);
  void on_pSpeedRotateSpinBox_valueChanged(double dValue);
  void on_pStopCheckBox_toggled(bool bChecked);
  void on_pConfirmButton_clicked();
  void on_pCancelButton_clicked();

private:
  std::unique_ptr<Ui::CDeviceSnippetOverlay> m_spUi;
  SDeviceSnippetData                         m_data;
  QSize                                      m_preferredSize;
};

#endif // DEVICESNIPPETOVERLAY_H
