#ifndef CSLIDINGSTACKEDWIDGET_H
#define CSLIDINGSTACKEDWIDGET_H

#include <QEasingCurve>
#include <QStackedWidget>
#include <chrono>

/*!
 * \brief Inspired by https://github.com/Qt-Widgets/Sliding-Stacked-Widget
 */
class CSlidingStackedWidget : public QStackedWidget
{
  Q_OBJECT
  Q_PROPERTY(QEasingCurve::Type animation READ Animation WRITE SetAnimation NOTIFY animationChanged)
  Q_PROPERTY(ESlideDirection slideDirection READ SlideDirection WRITE SetSlideDirection NOTIFY slideDirectionChanged)
  Q_PROPERTY(qint32 speed READ Speed WRITE SetSpeed NOTIFY speedChanged)
  Q_PROPERTY(bool wrap READ Wrap WRITE SetWrap NOTIFY wrapChanged)

public:
  enum ESlideDirection
  {
    eLeft2Right,
    eRight2Left,
    eTop2Bottom,
    eBottom2Top
  };
  Q_ENUM(ESlideDirection)

  explicit CSlidingStackedWidget(QWidget* pParent = nullptr);
  ~CSlidingStackedWidget() override;

  void SetAnimation(QEasingCurve::Type animationtype);
  QEasingCurve::Type Animation() const;

  void SetSlideDirection(ESlideDirection slideDirection);
  ESlideDirection SlideDirection() const;

  void SetSpeed(qint32 iSpeedMs);
  qint32 Speed() const;

  void SetWrap(bool bWrap);
  bool Wrap() const;

public slots:
  void SlideInNext();
  void SlideInPrev();
  void SlideInIdx(qint32 idx);

signals:
  void animationChanged();
  void animationFinished();
  void slideDirectionChanged();
  void speedChanged();
  void wrapChanged();

protected slots:
  void AnimationDoneSlot();

protected:
  void SlideInWgt(QWidget* pWidget, ESlideDirection direction);

private:
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

#endif // CSLIDINGSTACKEDWIDGET_H
