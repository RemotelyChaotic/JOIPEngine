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
void CSearchWidget::SetFilter(const QString& sFilter)
{
  m_spUi->pFilterLineEdit->setText(sFilter);
}

//----------------------------------------------------------------------------------------
//
QString CSearchWidget::Filter()
{
  return m_spUi->pFilterLineEdit->text();
}

//----------------------------------------------------------------------------------------
//
void CSearchWidget::SetFocus()
{
  m_spUi->pFilterLineEdit->setFocus();
}

//----------------------------------------------------------------------------------------
//
void CSearchWidget::on_pFilterLineEdit_editingFinished()
{
  emit SignalFilterChanged(Filter());
}

//----------------------------------------------------------------------------------------
//
void CSearchWidget::on_pFilterLineEdit_textChanged(const QString& sText)
{
  emit SignalFilterChanged(sText);
}
