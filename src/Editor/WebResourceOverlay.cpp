#include "WebResourceOverlay.h"
#include "ui_WebResourceOverlay.h"

CWebResourceOverlay::CWebResourceOverlay(QPushButton* pButtonTarget, QWidget* pParent) :
  COverlayBase(pParent),
  m_spUi(new Ui::CWebResourceOverlay),
  m_pButtonTarget(pButtonTarget)
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
  if (nullptr != m_pButtonTarget)
  {
    QPoint newPos = m_pButtonTarget->pos() +
        QPoint(m_pButtonTarget->size().width() / 2, m_pButtonTarget->size().height() + 10) -
        QPoint(size().width() / 2, 0);
    move(newPos.x(), newPos.y());
  }
}

//----------------------------------------------------------------------------------------
//
void CWebResourceOverlay::on_pConfirmButton_clicked()
{
  const QString sResource = m_spUi->pUrlLineEdit->text();
  emit SignalResourceSelected(sResource);
}
