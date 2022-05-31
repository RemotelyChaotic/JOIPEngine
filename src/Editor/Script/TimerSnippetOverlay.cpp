#include "TimerSnippetOverlay.h"
#include "ScriptEditorWidget.h"
#include "ui_TimerSnippetOverlay.h"

#include <QScrollBar>

CTimerSnippetOverlay::CTimerSnippetOverlay(CScriptEditorWidget* pParent) :
  COverlayBase(0, pParent),
  m_spUi(new Ui::CTimerSnippetOverlay),
  m_pEditor(pParent),
  m_data()
{
  m_spUi->setupUi(this);
  m_spUi->pScrollArea->setWidgetResizable(true);
  m_preferredSize = size();
}

CTimerSnippetOverlay::~CTimerSnippetOverlay()
{
}

//----------------------------------------------------------------------------------------
//
void CTimerSnippetOverlay::Climb()
{
  ClimbToFirstInstanceOf("QStackedWidget", false);
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
  QString sCode;

  if (m_data.m_bStop)
  {
    QString sTimer("timer.stop();\n");
    sCode += sTimer;
  }
  if (m_data.m_bHide)
  {
    QString sTimer("timer.hide();\n");
    sCode += sTimer;
  }

  QString sTimerVisible("timer.setTimeVisible(%1);\n");
  sCode += sTimerVisible.arg(m_data.m_bTimerVisible ? "true" : "false");

  if (m_data.m_bSetTime)
  {
    QString sTimer("timer.setTime(%1);\n");
    sCode += sTimer.arg(m_data.m_iTimeS);
  }
  if (m_data.m_bShow)
  {
    QString sTimer("timer.show();\n");
    sCode += sTimer;
  }
  if (m_data.m_bStart)
  {
    QString sTimer("timer.start();\n");
    sCode += sTimer;
  }
  if (m_data.m_bWait)
  {
    QString sTimer("timer.waitForTimer();\n");
    sCode += sTimer;
  }

  emit SignalTimerCode(sCode);
  Hide();
}

//----------------------------------------------------------------------------------------
//
void CTimerSnippetOverlay::on_pCancelButton_clicked()
{
  Hide();
}
