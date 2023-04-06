#include "TimerSnippetOverlay.h"
#include "ui_TimerSnippetOverlay.h"

#include <QScrollBar>

CTimerSnippetOverlay::CTimerSnippetOverlay(QWidget* pParent) :
  CCodeSnippetOverlayBase(pParent),
  m_spUi(new Ui::CTimerSnippetOverlay),
  m_data()
{
  m_spUi->setupUi(this);
  m_spUi->pScrollArea->setWidgetResizable(true);
  m_preferredSize = size();
  m_bInitialized = true;
}

CTimerSnippetOverlay::~CTimerSnippetOverlay()
{
}

//----------------------------------------------------------------------------------------
//
void CTimerSnippetOverlay::Resize()
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

  m_spUi->pScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_spUi->pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  m_spUi->pScrollArea->widget()->setMinimumWidth(
        newSize.width() - m_spUi->pScrollArea->verticalScrollBar()->width() -
        m_spUi->pScrollArea->widget()->layout()->spacing());
}

//----------------------------------------------------------------------------------------
//
void CTimerSnippetOverlay::on_pSetTimeCheckBox_toggled(bool bStatus)
{
  m_data.m_bSetTime = bStatus;
}

//----------------------------------------------------------------------------------------
//
void CTimerSnippetOverlay::on_pTimeEdit_timeChanged(const QTime &time)
{
  m_data.m_iTimeS = time.minute() * 60 + time.second();
}

//----------------------------------------------------------------------------------------
//
void CTimerSnippetOverlay::on_pShowCheckBox_toggled(bool bStatus)
{
  m_data.m_bShow = bStatus;
}

//----------------------------------------------------------------------------------------
//
void CTimerSnippetOverlay::on_pHideCheckBox_toggled(bool bStatus)
{
  m_data.m_bHide = bStatus;
}

//----------------------------------------------------------------------------------------
//
void CTimerSnippetOverlay::on_pDisplayCheckBox_toggled(bool bStatus)
{
  m_data.m_bTimerVisible = bStatus;
}

//----------------------------------------------------------------------------------------
//
void CTimerSnippetOverlay::on_pStartCheckBox_toggled(bool bStatus)
{
  m_data.m_bStart = bStatus;
}

//----------------------------------------------------------------------------------------
//
void CTimerSnippetOverlay::on_pStopCheckBox_toggled(bool bStatus)
{
  m_data.m_bStop = bStatus;
}

//----------------------------------------------------------------------------------------
//
void CTimerSnippetOverlay::on_pWaitCheckBox_toggled(bool bStatus)
{
  m_data.m_bWait = bStatus;
}

//----------------------------------------------------------------------------------------
//
void CTimerSnippetOverlay::on_pConfirmButton_clicked()
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
void CTimerSnippetOverlay::on_pCancelButton_clicked()
{
  Hide();
}
