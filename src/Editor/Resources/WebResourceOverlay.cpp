#include "WebResourceOverlay.h"
#include "ui_WebResourceOverlay.h"

CWebResourceOverlay::CWebResourceOverlay(QWidget* pParent) :
  COverlayBase(pParent),
  m_spUi(new Ui::CWebResourceOverlay)
{
  m_spUi->setupUi(this);
}

CWebResourceOverlay::~CWebResourceOverlay()
{
}

//----------------------------------------------------------------------------------------
//
void CWebResourceOverlay::Resize()
{
  QPoint newPos =
      QPoint(m_pTarget->geometry().width() / 2, m_pTarget->geometry().height() / 2) -
      QPoint(width() / 2, height() / 2);

  move(newPos.x(), newPos.y());
  resize(width(), height());
}

//----------------------------------------------------------------------------------------
//
void CWebResourceOverlay::on_pConfirmButton_clicked()
{
  const QString sResource = m_spUi->pUrlLineEdit->text();
  emit SignalResourceSelected(sResource);
}

//----------------------------------------------------------------------------------------
//
void CWebResourceOverlay::on_pCancelButton_clicked()
{
  Hide();
}
