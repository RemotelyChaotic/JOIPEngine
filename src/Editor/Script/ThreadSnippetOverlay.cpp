#include "ThreadSnippetOverlay.h"
#include "ui_ThreadSnippetOverlay.h"

CThreadSnippetOverlay::CThreadSnippetOverlay(QWidget* pParent) :
  COverlayBase(0, pParent),
  m_spUi(std::make_unique<Ui::CThreadSnippetOverlay>())
{
  m_spUi->setupUi(this);
  m_preferredSize = size();
}

CThreadSnippetOverlay::~CThreadSnippetOverlay()
{
}

//----------------------------------------------------------------------------------------
//
void CThreadSnippetOverlay::Climb()
{
  ClimbToFirstInstanceOf("QStackedWidget", false);
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
  m_bSleepTimeS = dValue;
}

//----------------------------------------------------------------------------------------
//
void CThreadSnippetOverlay::on_pSkippableCheckBox_toggled(bool bStatus)
{
  m_bSkippable = bStatus;
}

//----------------------------------------------------------------------------------------
//
void CThreadSnippetOverlay::on_pConfirmButton_clicked()
{
  QString sCode = QString("thread.sleep(%1,%2);\n")
      .arg(m_bSleepTimeS)
      .arg(m_bSkippable ? "true" : "false");
  emit SignalThreadCode(sCode);
  Hide();
}

//----------------------------------------------------------------------------------------
//
void CThreadSnippetOverlay::on_pCancelButton_clicked()
{
  Hide();
}
