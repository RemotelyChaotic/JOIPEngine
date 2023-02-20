#ifndef CSLIDINGWIDGET_H
#define CSLIDINGWIDGET_H

#include <QEasingCurve>
#include <QWidget>
#include <chrono>

class QParallelAnimationGroup;

/*!
 * \brief Inspired by https://github.com/Qt-Widgets/Sliding-Stacked-Widget
 */
class CSlidingWidget
{
public:
  enum ESlideDirection
  {
    eLeft2Right,
    eRight2Left,
    eTop2Bottom,
    eBottom2Top,
    eHorizontal,
    eVertical
  };

  CSlidingWidget();
  virtual ~CSlidingWidget();

  void SetAnimation(QEasingCurve::Type animationtype);
  QEasingCurve::Type Animation() const;

  void SetSlideDirection(ESlideDirection slideDirection);
  ESlideDirection SlideDirection() const;

  void SetSpeed(qint32 iSpeedMs);
  qint32 Speed() const;

  void SetWrap(bool bWrap);
  bool Wrap() const;

  virtual qint32 Count() const = 0;
  virtual qint32 CurrentIndex() const = 0;
  virtual qint32 IndexOf(QWidget* pWidget) const = 0;
  virtual QWidget* Widget(qint32 idx) const = 0;

  virtual void SetCurrentIndex(qint32 idx) = 0;

signals:
  virtual void animationChanged() = 0;
  virtual void animationFinished() = 0;
  virtual void slideDirectionChanged() = 0;
  virtual void speedChanged() = 0;
  virtual void wrapChanged() = 0;

protected:
  void AnimationDoneImpl(QParallelAnimationGroup* pAnimgroup);
  void SlideInNextImpl();
  void SlideInPrevImpl();
  void SlideInIdxImpl(qint32 idx);
  void SlideInWgt(QWidget* pWidget, ESlideDirection direction);

  QList<QWidget*> m_vBlockedPageList;
  QEasingCurve::Type m_animationType;
  ESlideDirection m_slideDirection;
  qint32 m_iSpeed;
  bool m_bWrap;
  QPoint m_pnow;
  int m_iNow;
  int m_iNext;
  bool m_bActive;
};

Q_DECLARE_INTERFACE(CSlidingWidget, "CSlidingWidget");

#endif // CSLIDINGWIDGET_H
