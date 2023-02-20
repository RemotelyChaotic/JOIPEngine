#include "SlidingWidget.h"

#include <QParallelAnimationGroup>
#include <QPointer>
#include <QPropertyAnimation>

CSlidingWidget::CSlidingWidget() :
  m_animationType(QEasingCurve::InOutQuad),
  m_slideDirection(ESlideDirection::eHorizontal),
  m_iSpeed(500),
  m_bWrap(true),
  m_pnow(0, 0),
  m_iNow(0),
  m_iNext(0),
  m_bActive(false)
{
}

CSlidingWidget::~CSlidingWidget()
{

}

//----------------------------------------------------------------------------------------
//
void CSlidingWidget::SetAnimation(QEasingCurve::Type animationtype)
{
  if (m_animationType != animationtype)
  {
    m_animationType = animationtype;
    emit animationChanged();
  }
}

//----------------------------------------------------------------------------------------
//
QEasingCurve::Type CSlidingWidget::Animation() const
{
  return m_animationType;
}

//----------------------------------------------------------------------------------------
//
void CSlidingWidget::SetSlideDirection(ESlideDirection slideDirection)
{
  if (m_slideDirection != slideDirection)
  {
    m_slideDirection = slideDirection;
    emit slideDirectionChanged();
  }
}

//----------------------------------------------------------------------------------------
//
CSlidingWidget::ESlideDirection CSlidingWidget::SlideDirection() const
{
  return m_slideDirection;
}

//----------------------------------------------------------------------------------------
//
void CSlidingWidget::SetSpeed(qint32 iSpeedMs)
{
  if (m_iSpeed != iSpeedMs)
  {
    m_iSpeed = iSpeedMs;
    emit speedChanged();
  }
}

//----------------------------------------------------------------------------------------
//
qint32 CSlidingWidget::Speed() const
{
  return m_iSpeed;
}

//----------------------------------------------------------------------------------------
//
void CSlidingWidget::SetWrap(bool bWrap)
{
  if (m_bWrap != bWrap)
  {
    m_bWrap = bWrap;
    emit wrapChanged();
  }
}

//----------------------------------------------------------------------------------------
//
bool CSlidingWidget::Wrap() const
{
  return m_bWrap;
}

//----------------------------------------------------------------------------------------
//
void CSlidingWidget::AnimationDoneImpl(QParallelAnimationGroup* pAnimgroup)
{
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
  SetCurrentIndex(m_iNext);
  Widget(m_iNow)->hide();
  Widget(m_iNow)->move(m_pnow);
  m_bActive = false;
  emit animationFinished();
}

//----------------------------------------------------------------------------------------
//
void CSlidingWidget::SlideInNextImpl()
{
  qint32 iNow  = CurrentIndex();
  if (m_bWrap || Count() - 1 > iNow)
  {
     SlideInIdxImpl(iNow + 1);
  }
}

//----------------------------------------------------------------------------------------
//
void CSlidingWidget::SlideInPrevImpl()
{
  int iNow = CurrentIndex();
  if (m_bWrap || 0 < iNow)
  {
    SlideInIdxImpl(iNow - 1);
  }
}

//----------------------------------------------------------------------------------------
//
void CSlidingWidget::SlideInIdxImpl(qint32 idx)
{
  if (Count() - 1 < idx)
  {
    idx = idx % Count();
  }
  else if (idx < 0)
  {
    idx = (idx + Count()) % Count();
  }
  SlideInWgt(Widget(idx), m_slideDirection);
}

//----------------------------------------------------------------------------------------
//
void CSlidingWidget::SlideInWgt(QWidget* pWidget, ESlideDirection direction)
{
  QPointer<QWidget> pThis = dynamic_cast<QWidget*>(this);

  if (m_bActive)
  {
    return;
  }
  else
  {
    m_bActive = true;
  }

  qint32 iNow  = CurrentIndex(); /* currentIndex() is a function inherited from QStackedWidget */
  qint32 iNext = IndexOf(pWidget);
  if (iNext == iNow)
  {
    m_bActive = false;
    return;
  }

  qint32 iOffsetx = pThis->width();
  qint32 iOffsety = pThis->height();
  Widget(iNext)->setGeometry(0, 0, iOffsetx, iOffsety);

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
    case ESlideDirection::eHorizontal:
    {
      if (iNow < iNext)
      {
        iOffsetx = -iOffsetx;
        iOffsety = 0;
      }
      else
      {
        iOffsety = 0;
      }
    } break;
    case ESlideDirection::eVertical:
    {
      if (iNow < iNext)
      {
        iOffsetx = 0;
        iOffsety = -iOffsety;
      }
      else
      {
        iOffsetx = 0;
      }
    } break;
  }

  QPoint pNext = Widget(iNext)->pos();
  QPoint pNow  = Widget(iNow)->pos();
  m_pnow = pNow;

  Widget(iNext)->move(pNext.x() - iOffsetx, pNext.y() - iOffsety);
  Widget(iNext)->show();
  Widget(iNext)->raise();

  QParallelAnimationGroup* pAnimgroup = new QParallelAnimationGroup(pThis);

  /* Animate both, the now and next widget to the side, using animation framework */
  QPropertyAnimation* pAnimnow = new QPropertyAnimation(Widget(iNow), "pos");
  pAnimnow->setDuration(m_iSpeed);
  pAnimnow->setEasingCurve(m_animationType);
  pAnimnow->setStartValue(QPoint(pNow.x(), pNow.y()));
  pAnimnow->setEndValue(QPoint(iOffsetx + pNow.x(), iOffsety + pNow.y()));
  QPropertyAnimation* pAnimnext = new QPropertyAnimation(Widget(iNext), "pos");
  pAnimnext->setDuration(m_iSpeed);
  pAnimnext->setEasingCurve(m_animationType);
  pAnimnext->setStartValue(QPoint(-iOffsetx + pNext.x(), -iOffsety + pNext.y()));
  pAnimnext->setEndValue(QPoint(pNext.x(), pNext.y()));

  pAnimgroup->addAnimation(pAnimnow);
  pAnimgroup->addAnimation(pAnimnext);

  QObject::connect(pAnimgroup, &QParallelAnimationGroup::finished,
          pThis, [pThis, pAnimgroup, this]() {
    if (nullptr != pThis && nullptr != pAnimgroup)
    {
      AnimationDoneImpl(pAnimgroup);
    }
  });

  m_iNext   = iNext;
  m_iNow    = iNow;
  m_bActive = true;
  pAnimgroup->start();
}
