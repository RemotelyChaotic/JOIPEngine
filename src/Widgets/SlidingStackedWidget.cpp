#include "SlidingStackedWidget.h"
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>

CSlidingStackedWidget::CSlidingStackedWidget(QWidget *parent)
  : QStackedWidget{parent},
    m_animationType(QEasingCurve::InOutQuad),
    m_slideDirection(ESlideDirection::eLeft2Right),
    m_iSpeed(500),
    m_bWrap(true),
    m_pnow(0, 0),
    m_iNow(0),
    m_iNext(0),
    m_bActive(false)
{
}

CSlidingStackedWidget::~CSlidingStackedWidget() = default;

//----------------------------------------------------------------------------------------
//
void CSlidingStackedWidget::SetAnimation(QEasingCurve::Type animationtype)
{
  if (m_animationType != animationtype)
  {
    m_animationType = animationtype;
    emit animationChanged();
  }
}

//----------------------------------------------------------------------------------------
//
QEasingCurve::Type CSlidingStackedWidget::Animation() const
{
  return m_animationType;
}

//----------------------------------------------------------------------------------------
//
void CSlidingStackedWidget::SetSlideDirection(ESlideDirection slideDirection)
{
  if (m_slideDirection != slideDirection)
  {
    m_slideDirection = slideDirection;
    emit slideDirectionChanged();
  }
}

//----------------------------------------------------------------------------------------
//
CSlidingStackedWidget::ESlideDirection CSlidingStackedWidget::SlideDirection() const
{
  return m_slideDirection;
}

//----------------------------------------------------------------------------------------
//
void CSlidingStackedWidget::SetSpeed(qint32 iSpeedMs)
{
  if (m_iSpeed != iSpeedMs)
  {
    m_iSpeed = iSpeedMs;
    emit speedChanged();
  }
}

//----------------------------------------------------------------------------------------
//
qint32 CSlidingStackedWidget::Speed() const
{
  return m_iSpeed;
}

//----------------------------------------------------------------------------------------
//
void CSlidingStackedWidget::SetWrap(bool bWrap)
{
  if (m_bWrap != bWrap)
  {
    m_bWrap = bWrap;
    emit wrapChanged();
  }
}

//----------------------------------------------------------------------------------------
//
bool CSlidingStackedWidget::Wrap() const
{
  return m_bWrap;
}

//----------------------------------------------------------------------------------------
//
void CSlidingStackedWidget::SlideInNext()
{
  qint32 iNow  = currentIndex();
  if (m_bWrap || count() - 1 > iNow)
  {
     SlideInIdx(iNow + 1);
  }
}

//----------------------------------------------------------------------------------------
//
void CSlidingStackedWidget::SlideInPrev()
{
  int iNow = currentIndex();
  if (m_bWrap || 0 < iNow)
  {
    SlideInIdx(iNow - 1);
  }
}

//----------------------------------------------------------------------------------------
//
void CSlidingStackedWidget::SlideInIdx(qint32 idx)
{
  if (count() - 1 < idx)
  {
    idx = idx % count();
  }
  else if (idx < 0)
  {
    idx = (idx + count()) % count();
  }
  SlideInWgt(widget(idx), m_slideDirection);
}

//----------------------------------------------------------------------------------------
//
void CSlidingStackedWidget::AnimationDoneSlot()
{
  QParallelAnimationGroup* pAnimgroup = dynamic_cast<QParallelAnimationGroup*>(sender());
  if (nullptr != pAnimgroup)
  {
    for (qint32 i = 0; pAnimgroup->animationCount() > i; ++i)
    {
      auto pAnim = pAnimgroup->animationAt(i);
      if (nullptr != pAnim)
      {
        pAnim->deleteLater();
      }
    }
    pAnimgroup->deleteLater();
  }

  /* When ready, call the QStackedWidget slot setCurrentIndex(int) */
  setCurrentIndex(m_iNext);
  widget(m_iNow)->hide();
  widget(m_iNow)->move(m_pnow);
  m_bActive = false;
  emit animationFinished();
}

//----------------------------------------------------------------------------------------
//
void CSlidingStackedWidget::SlideInWgt(QWidget* pWidget, ESlideDirection direction)
{
  if (m_bActive)
  {
    return;
  }
  else
  {
    m_bActive = true;
  }

  qint32 iNow  = currentIndex(); /* currentIndex() is a function inherited from QStackedWidget */
  qint32 iNext = indexOf(pWidget);
  if (iNext == iNow)
  {
    m_bActive = false;
    return;
  }

  qint32 iOffsetx = frameRect().width();
  qint32 iOffsety = frameRect().height();
  widget(iNext)->setGeometry(0, 0, iOffsetx, iOffsety);

  switch (direction)
  {
    case ESlideDirection::eBottom2Top:
    {
      iOffsetx = 0;
      iOffsety = -iOffsety;
    } break;
    case ESlideDirection::eTop2Bottom:
    {
      iOffsetx = 0;
    } break;
    case ESlideDirection::eRight2Left:
    {
      iOffsetx = -iOffsetx;
      iOffsety = 0;
    } break;
    case ESlideDirection::eLeft2Right:
    {
      iOffsety = 0;
    } break;
  }

  QPoint pNext = widget(iNext)->pos();
  QPoint pNow  = widget(iNow)->pos();
  m_pnow = pNow;

  widget(iNext)->move(pNext.x() - iOffsetx, pNext.y() - iOffsety);
  widget(iNext)->show();
  widget(iNext)->raise();

  QParallelAnimationGroup* pAnimgroup = new QParallelAnimationGroup(this);

  /* Animate both, the now and next widget to the side, using animation framework */
  QPropertyAnimation* pAnimnow = new QPropertyAnimation(widget(iNow), "pos");
  pAnimnow->setDuration(m_iSpeed);
  pAnimnow->setEasingCurve(m_animationType);
  pAnimnow->setStartValue(QPoint(pNow.x(), pNow.y()));
  pAnimnow->setEndValue(QPoint(iOffsetx + pNow.x(), iOffsety + pNow.y()));
  QPropertyAnimation* pAnimnext = new QPropertyAnimation(widget(iNext), "pos");
  pAnimnext->setDuration(m_iSpeed);
  pAnimnext->setEasingCurve(m_animationType);
  pAnimnext->setStartValue(QPoint(-iOffsetx + pNext.x(), iOffsety + pNext.y()));
  pAnimnext->setEndValue(QPoint(pNext.x(), pNext.y()));

  pAnimgroup->addAnimation(pAnimnow);
  pAnimgroup->addAnimation(pAnimnext);

  connect(pAnimgroup, &QParallelAnimationGroup::finished,
          this, &CSlidingStackedWidget::AnimationDoneSlot, Qt::UniqueConnection);

  m_iNext   = iNext;
  m_iNow    = iNow;
  m_bActive = true;
  pAnimgroup->start();
}
