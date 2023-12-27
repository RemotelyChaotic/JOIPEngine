#include "SearchWidget.h"
#include "ui_SearchWidget.h"
#include "Utils/UndoRedoFilter.h"

CSearchWidget::CSearchWidget(QWidget* pParent) :
  QWidget(pParent),
  m_spUi(std::make_unique<Ui::CSearchWidget>()),
  m_pUndoFilter(nullptr)
{
  m_spUi->setupUi(this);
  m_spUi->pFilterLineEdit->setPlaceholderText(QStringLiteral("Filter"));
  m_spUi->pFilterLineEdit->setClearButtonEnabled(true);
}

CSearchWidget::~CSearchWidget()
{
}

//----------------------------------------------------------------------------------------
//
void CSearchWidget::SetFilterUndo(bool bValue)
{
  if (IsFilterUndoSet() != bValue)
  {
    if (!bValue)
    {
      m_pUndoFilter->setParent(nullptr);
      delete m_pUndoFilter;
      m_pUndoFilter = nullptr;
    }
    else
    {
      m_pUndoFilter = new CUndoRedoFilter(m_spUi->pFilterLineEdit, nullptr);
    }
  }
}

//----------------------------------------------------------------------------------------
//
bool CSearchWidget::IsFilterUndoSet() const
{
  return nullptr != m_pUndoFilter;
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
