#include "DeviceSnippetOverlay.h"
#include "ui_DeviceSnippetOverlay.h"

CDeviceSnippetOverlay::CDeviceSnippetOverlay(QWidget* pParent) :
  CCodeSnippetOverlayBase(pParent),
  m_spUi(std::make_unique<Ui::CDeviceSnippetOverlay>())
{
  m_spUi->setupUi(this);
  m_preferredSize = size();
  m_bInitialized = true;
}
CDeviceSnippetOverlay::~CDeviceSnippetOverlay()
{
}

//----------------------------------------------------------------------------------------
//
void CDeviceSnippetOverlay::Resize()
{
  QSize newSize = m_preferredSize;
  if (m_pTargetWidget->geometry().width() < m_preferredSize.width())
  {
    newSize.setWidth(m_pTargetWidget->geometry().width());
  }
  if (m_pTargetWidget->geometry().height() < m_preferredSize.height())
  {
    newSize.setHeight(m_pTargetWidget->geometry().height());
  }

  QPoint newPos =
      QPoint(m_pTargetWidget->geometry().width() / 2, m_pTargetWidget->geometry().height() / 2) -
      QPoint(newSize.width() / 2, newSize.height() / 2);

  move(newPos.x(), newPos.y());
  resize(newSize);
}

//----------------------------------------------------------------------------------------
//
void CDeviceSnippetOverlay::on_pVibrateGroupBox_toggled(bool bChecked)
{
  m_data.m_bVibrateCommand = bChecked;
}

//----------------------------------------------------------------------------------------
//
void CDeviceSnippetOverlay::on_pSpeedVibrateSpinBox_valueChanged(double dValue)
{
  m_data.m_dVibrateSpeed = dValue / 100.0;
}

//----------------------------------------------------------------------------------------
//
void CDeviceSnippetOverlay::on_pLinearGroupBox_toggled(bool bChecked)
{
  m_data.m_bLinearCommand = bChecked;
}

//----------------------------------------------------------------------------------------
//
void CDeviceSnippetOverlay::on_pDurationSpinBox_valueChanged(double dValue)
{
  m_data.m_dLinearDurationS = dValue;
}

//----------------------------------------------------------------------------------------
//
void CDeviceSnippetOverlay::on_pPositionSpinBox_valueChanged(double dValue)
{
  m_data.m_dLinearPosition = dValue / 100.0;
}

//----------------------------------------------------------------------------------------
//
void CDeviceSnippetOverlay::on_pRotateGroupBox_toggled(bool bChecked)
{
  m_data.m_bRotateCommand = bChecked;
}

//----------------------------------------------------------------------------------------
//
void CDeviceSnippetOverlay::on_pClockwiseCheckBox_toggled(bool bChecked)
{
  m_data.m_bClockwiseRotate = bChecked;
}

//----------------------------------------------------------------------------------------
//
void CDeviceSnippetOverlay::on_pSpeedRotateSpinBox_valueChanged(double dValue)
{
  m_data.m_dRotateSpeed = dValue / 100.0;
}

//----------------------------------------------------------------------------------------
//
void CDeviceSnippetOverlay::on_pStopCheckBox_toggled(bool bChecked)
{
  m_data.m_bStopCommand = bChecked;
}

//----------------------------------------------------------------------------------------
//
void CDeviceSnippetOverlay::on_pConfirmButton_clicked()
{
  auto spGenerator = CodeGenerator();
  if (nullptr != spGenerator)
  {
    emit SignalCodeGenerated(spGenerator->Generate(m_data, nullptr));
  }
  Hide();
}

//----------------------------------------------------------------------------------------
//
void CDeviceSnippetOverlay::on_pCancelButton_clicked()
{
  Hide();
}
