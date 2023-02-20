#include "SlidingStackedWidget.h"

#include <QParallelAnimationGroup>

CSlidingStackedWidget::CSlidingStackedWidget(QWidget *parent)
  : QStackedWidget{parent},
    CSlidingWidget()
{
}

CSlidingStackedWidget::~CSlidingStackedWidget() = default;

//----------------------------------------------------------------------------------------
//
qint32 CSlidingStackedWidget::Count() const
{
  return count();
}

//----------------------------------------------------------------------------------------
//
qint32 CSlidingStackedWidget::CurrentIndex() const
{
  return currentIndex();
}

//----------------------------------------------------------------------------------------
//
qint32 CSlidingStackedWidget::IndexOf(QWidget* pWidget) const
{
  return indexOf(pWidget);
}

//----------------------------------------------------------------------------------------
//
QWidget* CSlidingStackedWidget::Widget(qint32 idx) const
{
  return widget(idx);
}

//----------------------------------------------------------------------------------------
//
void CSlidingStackedWidget::SetCurrentIndex(qint32 idx)
{
  setCurrentIndex(idx);
}

//----------------------------------------------------------------------------------------
//
void CSlidingStackedWidget::SlideInNext()
{
  SlideInNextImpl();
}

//----------------------------------------------------------------------------------------
//
void CSlidingStackedWidget::SlideInPrev()
{
  SlideInPrevImpl();
}

//----------------------------------------------------------------------------------------
//
void CSlidingStackedWidget::SlideInIdx(qint32 idx)
{
  SlideInIdxImpl(idx);
}
