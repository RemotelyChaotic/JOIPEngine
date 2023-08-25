#include "CompressJobSettingsOverlay.h"
#include "ui_CompressJobSettingsOverlay.h"

CCompressJobSettingsOverlay::CCompressJobSettingsOverlay(QWidget* pParent) :
    COverlayBase(100, pParent),
    m_spUi(std::make_unique<Ui::CCompressJobSettingsOverlay>())
{
  qRegisterMetaType<CCompressJobSettingsOverlay::SCompressJobSettings>();
  m_spUi->setupUi(this);
}

CCompressJobSettingsOverlay::~CCompressJobSettingsOverlay()
{
}

//----------------------------------------------------------------------------------------
//
void CCompressJobSettingsOverlay::Climb()
{
  ClimbToFirstInstanceOf("CEditorMainScreen", false);
}

//----------------------------------------------------------------------------------------
//
void CCompressJobSettingsOverlay::Resize()
{
  QPoint newPos =
      QPoint(m_pTargetWidget->geometry().width() / 2, m_pTargetWidget->geometry().height() / 2) -
      QPoint(width() / 2, height() / 2);

  move(newPos.x(), newPos.y());
  resize(width(), height());
}

//----------------------------------------------------------------------------------------
//
void CCompressJobSettingsOverlay::on_pConfirmButton_clicked()
{
  SCompressJobSettings settings{ m_spUi->pCompressionSlider->value() };
  emit SignalJobSettingsConfirmed(settings);
  Hide();
}

//----------------------------------------------------------------------------------------
//
void CCompressJobSettingsOverlay::on_pCancelButton_clicked()
{
  Hide();
}
