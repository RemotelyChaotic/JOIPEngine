#include "TimerDisplayWidget.h"
#include "Application.h"
#include "Settings.h"
#include "ui_TimerDisplayWidget.h"

#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QTimer>

namespace {
  const qint32 c_iUpdateInterval = 10;
}

CTimerDisplayWidget::CTimerDisplayWidget(QWidget* pParent) :
  QWidget(pParent),
  IWidgetBaseInterface(),
  m_spUi(std::make_unique<Ui::CTimerDisplayWidget>()),
  m_spTimer(std::make_unique<QTimer>()),
  m_spSettings(CApplication::Instance()->Settings()),
  m_bTimerShown(false)
{
  m_spUi->setupUi(this);

  m_spTimer->setSingleShot(false);
  m_spTimer->setInterval(c_iUpdateInterval);
  connect(m_spTimer.get(), &QTimer::timeout,
          m_spUi->pTimer, &CTimerWidget::Update);

  m_spUi->pTimer->hide();
  m_spUi->pTimer->SetUpdateInterval(c_iUpdateInterval);

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
  if (m_bTimerShown)
  {
    m_bTimerShown = false;

    RemoveEffect(m_spUi->pTimer);

    QGraphicsOpacityEffect* pOpacity = new QGraphicsOpacityEffect(m_spUi->pTimer);
    pOpacity->setOpacity(1.0);
    m_spUi->pTimer->setGraphicsEffect(pOpacity);
    QPropertyAnimation* pTimerAnimation = new QPropertyAnimation(pOpacity, "opacity", pOpacity);
    pTimerAnimation->setDuration(200);
    pTimerAnimation->setStartValue(1.0);
    pTimerAnimation->setEndValue(0.0);
    connect(pTimerAnimation, &QPropertyAnimation::finished, this, &CTimerDisplayWidget::SlotTimerAnimationFinished);
    pTimerAnimation->start();

    emit SignalTimerFinished();
  }
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
  if (!m_bTimerShown)
  {
    m_bTimerShown = true;

    RemoveEffect(m_spUi->pTimer);

    QGraphicsOpacityEffect* pOpacity = new QGraphicsOpacityEffect(m_spUi->pTimer);
    pOpacity->setOpacity(0.00);
    m_spUi->pTimer->setGraphicsEffect(pOpacity);
    QPropertyAnimation* pTimerAnimation = new QPropertyAnimation(pOpacity, "opacity", pOpacity);
    pTimerAnimation->setDuration(200);
    pTimerAnimation->setStartValue(0.0);
    pTimerAnimation->setEndValue(1.0);
    connect(pTimerAnimation, &QPropertyAnimation::finished, this, &CTimerDisplayWidget::SlotTimerAnimationFinished);

    m_spUi->pTimer->show();
    pTimerAnimation->start();
  }
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
  RemoveEffect(pWidget);

  QGraphicsDropShadowEffect* pShadow = new QGraphicsDropShadowEffect(pWidget);
  pShadow->setBlurRadius(5);
  pShadow->setXOffset(5);
  pShadow->setYOffset(5);
  pShadow->setColor(Qt::black);
  pWidget->setGraphicsEffect(pShadow);
}

//----------------------------------------------------------------------------------------
//
void CTimerDisplayWidget::RemoveEffect(QWidget* pWidget)
{
  if (nullptr != pWidget)
  {
    QGraphicsEffect* pEffect = pWidget->graphicsEffect();
    if (nullptr != pEffect)
    {
      delete pEffect;
    }

    QObjectList children = pWidget->children();
    for (QObject* pObj : children)
    {
      QWidget* pWidget = dynamic_cast<QWidget*>(pObj);
      if (nullptr != pWidget)
      {
        pEffect = pWidget->graphicsEffect();
        if (nullptr != pEffect)
        {
          delete pEffect;
        }
      }
    }
  }

  repaint();
}

//----------------------------------------------------------------------------------------
//
void CTimerDisplayWidget::SlotTimerAnimationFinished()
{
  if (m_bTimerShown)
  {
    AddDropShadow(m_spUi->pTimer);
  }
  else
  {
    RemoveEffect(m_spUi->pTimer);
    m_spUi->pTimer->hide();
  }
}
