#include "ThreadSnippetOverlay.h"
#include "ui_ThreadSnippetOverlay.h"

CThreadSnippetOverlay::CThreadSnippetOverlay(QWidget* pParent) :
  CCodeSnippetOverlayBase(pParent),
  m_spUi(std::make_unique<Ui::CThreadSnippetOverlay>()),
  m_data()
{
  m_spUi->setupUi(this);
  m_preferredSize = size();
  m_bInitialized = true;
}

CThreadSnippetOverlay::~CThreadSnippetOverlay()
{
}

//----------------------------------------------------------------------------------------
//
void CThreadSnippetOverlay::Resize()
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
void CThreadSnippetOverlay::on_pSleepSpinBox_valueChanged(double dValue)
{
  m_data.m_bSleepTimeS = dValue;
}

//----------------------------------------------------------------------------------------
//
void CThreadSnippetOverlay::on_pSkippableCheckBox_toggled(bool bStatus)
{
  m_data.m_bSkippable = bStatus;
}

//----------------------------------------------------------------------------------------
//
void CThreadSnippetOverlay::on_pConfirmButton_clicked()
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
void CThreadSnippetOverlay::on_pCancelButton_clicked()
{
  Hide();
}
