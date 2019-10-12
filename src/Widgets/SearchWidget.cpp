#include "SearchWidget.h"
#include "ui_SearchWidget.h"

CSearchWidget::CSearchWidget(QWidget* pParent) :
  QWidget(pParent),
  m_spUi(std::make_unique<Ui::CSearchWidget>())
{
  m_spUi->setupUi(this);
}

CSearchWidget::~CSearchWidget()
{
}

//----------------------------------------------------------------------------------------
//
void CSearchWidget::on_pFilterLineEdit_textChanged(const QString& sText)
{
  emit SignalFilterChanged(sText);
}
