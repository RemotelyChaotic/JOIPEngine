#include "TimerDisplayWidget.h"
#include "Application.h"
#include "Settings.h"
#include "ui_TimerDisplayWidget.h"

#include <QGraphicsDropShadowEffect>
#include <QTimer>

namespace {
  const qint32 c_iUpdateInterval = 10;
}

CTimerDisplayWidget::CTimerDisplayWidget(QWidget* pParent) :
  QWidget(pParent),
  IWidgetBaseInterface(),
  m_spUi(std::make_unique<Ui::CTimerDisplayWidget>()),
  m_spTimer(std::make_unique<QTimer>()),
  m_spSettings(CApplication::Instance()->Settings())
{
  m_spUi->setupUi(this);

  m_spTimer->setSingleShot(false);
  m_spTimer->setInterval(c_iUpdateInterval);
  connect(m_spTimer.get(), &QTimer::timeout,
          m_spUi->pTimer, &CTimerWidget::Update);

  m_spUi->pTimer->hide();
  m_spUi->pTimer->SetUpdateInterval(c_iUpdateInterval);

  AddDropShadow(m_spUi->pTimer);

  connect(m_spUi->pTimer, &CTimerWidget::SignalTimerFinished,
          this, &CTimerDisplayWidget::SlotHideTimer);
}

CTimerDisplayWidget::~CTimerDisplayWidget()
{
}

//----------------------------------------------------------------------------------------
//
void CTimerDisplayWidget::Initialize()
{
  m_bInitialized = false;

  // initializing done
  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CTimerDisplayWidget::SlotHideTimer()
{
  m_spUi->pTimer->hide();
  emit SignalTimerFinished();
}

//----------------------------------------------------------------------------------------
//
void CTimerDisplayWidget::SlotSetTime(qint32 iTimeS)
{
  m_spUi->pTimer->SetTimer(iTimeS * 1000);
}

//----------------------------------------------------------------------------------------
//
void CTimerDisplayWidget::SlotSetTimeVisible(bool bVisible)
{
  m_spUi->pTimer->SetTimerVisible(bVisible);
}

//----------------------------------------------------------------------------------------
//
void CTimerDisplayWidget::SlotShowTimer()
{
  m_spUi->pTimer->show();
}

//----------------------------------------------------------------------------------------
//
void CTimerDisplayWidget::SlotStartTimer()
{
  m_spTimer->start();
}

//----------------------------------------------------------------------------------------
//
void CTimerDisplayWidget::SlotStopTimer()
{
  m_spTimer->stop();
}

//----------------------------------------------------------------------------------------
//
void CTimerDisplayWidget::SlotWaitForTimer()
{
  if (!m_spTimer->isActive())
  {
    emit SignalTimerFinished();
  }
}

//----------------------------------------------------------------------------------------
//
void CTimerDisplayWidget::AddDropShadow(QWidget* pWidget)
{
  QGraphicsDropShadowEffect* pShadow = new QGraphicsDropShadowEffect(pWidget);
  pShadow->setBlurRadius(5);
  pShadow->setXOffset(5);
  pShadow->setYOffset(5);
  pShadow->setColor(Qt::black);
  pWidget->setGraphicsEffect(pShadow);
}
