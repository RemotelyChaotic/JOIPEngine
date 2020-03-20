#include "TimerSnippetOverlay.h"
#include "ui_TimerSnippetOverlay.h"

CTimerSnippetOverlay::CTimerSnippetOverlay(QWidget* pParent) :
  COverlayBase(pParent),
  m_spUi(new Ui::CTimerSnippetOverlay),
  m_data()
{
  m_spUi->setupUi(this);
}

CTimerSnippetOverlay::~CTimerSnippetOverlay()
{
}

//----------------------------------------------------------------------------------------
//
void CTimerSnippetOverlay::Climb()
{
  ClimbToFirstInstanceOf("CEditorMainScreen", false);
}


//----------------------------------------------------------------------------------------
//
void CTimerSnippetOverlay::Resize()
{
  QPoint newPos =
      QPoint(m_pTargetWidget->geometry().width() / 2, m_pTargetWidget->geometry().height() / 2) -
      QPoint(width() / 2, height() / 2);

  move(newPos.x(), newPos.y());
  resize(width(), height());
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
