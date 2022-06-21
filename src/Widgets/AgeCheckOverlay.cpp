#include "AgeCheckOverlay.h"
#include "ui_AgeCheckOverlay.h"

#include <QApplication>

CAgeCheckOverlay::CAgeCheckOverlay(QWidget* pParent) :
  COverlayBase(INT_MAX, pParent),
  m_spUi(std::make_unique<Ui::CAgeCheckOverlay>())
{
  m_spUi->setupUi(this);
}

CAgeCheckOverlay::~CAgeCheckOverlay()
{

}

//----------------------------------------------------------------------------------------
//
void CAgeCheckOverlay::Climb()
{
  ClimbToTop();
}

//----------------------------------------------------------------------------------------
//
void CAgeCheckOverlay::Resize()
{
  COverlayBase::Resize();
}

//----------------------------------------------------------------------------------------
//
void CAgeCheckOverlay::on_pConfirmButton_clicked()
{
  Hide();
}

//----------------------------------------------------------------------------------------
//
void CAgeCheckOverlay::on_pCancelButton_clicked()
{
  qApp->quit();
}
