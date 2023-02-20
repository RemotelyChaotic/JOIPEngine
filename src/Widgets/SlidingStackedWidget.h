#ifndef CSLIDINGSTACKEDWIDGET_H
#define CSLIDINGSTACKEDWIDGET_H

#include "SlidingWidget.h"

#include <QEasingCurve>
#include <QStackedWidget>
#include <chrono>

class CSlidingStackedWidget : public QStackedWidget, public CSlidingWidget
{
  Q_OBJECT
  Q_INTERFACES(CSlidingWidget)

  Q_PROPERTY(QEasingCurve::Type animation READ Animation WRITE SetAnimation NOTIFY animationChanged)
  Q_PROPERTY(ESlideDirection slideDirection READ SlideDirection WRITE SetSlideDirection NOTIFY slideDirectionChanged)
  Q_PROPERTY(qint32 speed READ Speed WRITE SetSpeed NOTIFY speedChanged)
  Q_PROPERTY(bool wrap READ Wrap WRITE SetWrap NOTIFY wrapChanged)

public:
  Q_ENUM(ESlideDirection)

  explicit CSlidingStackedWidget(QWidget* pParent = nullptr);
  ~CSlidingStackedWidget() override;

  qint32 Count() const override;
  qint32 CurrentIndex() const override;
  qint32 IndexOf(QWidget* pWidget) const override;
  QWidget* Widget(qint32 idx) const override;

  void SetCurrentIndex(qint32 idx) override;

public slots:
  void SlideInNext();
  void SlideInPrev();
  void SlideInIdx(qint32 idx);

signals:
  void animationChanged() override;
  void animationFinished() override;
  void slideDirectionChanged() override;
  void speedChanged() override;
  void wrapChanged() override;
};

#endif // CSLIDINGSTACKEDWIDGET_H
