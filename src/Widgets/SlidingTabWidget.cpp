#include "SlidingTabWidget.h"

#include <QStackedWidget>
#include <QTabBar>

CSlidingTabWidget::CSlidingTabWidget(QWidget *parent)
  : QTabWidget{parent},
    CSlidingWidget()
{
  m_pStack = findChild<QStackedWidget*>("qt_tabwidget_stackedwidget");

  QTabBar* pTabBar = tabBar();
  pTabBar->disconnect();
  connect(pTabBar, &QTabBar::currentChanged,
          this, &CSlidingTabWidget::SlideInIdx);
  connect(pTabBar, &QTabBar::tabBarClicked,
          this, &CSlidingTabWidget::tabBarClicked);
  connect(pTabBar, &QTabBar::tabBarDoubleClicked,
          this, &CSlidingTabWidget::tabBarDoubleClicked);
}

CSlidingTabWidget::~CSlidingTabWidget() = default;

//----------------------------------------------------------------------------------------
//
int CSlidingTabWidget::addTab(QWidget* pWidget, const QString& sName)
{
  qint32 iRet = QTabWidget::addTab(pWidget, sName);
  InsertTabImpl(count()-1);
  return iRet;
}

//----------------------------------------------------------------------------------------
//
int CSlidingTabWidget::addTab(QWidget* pWidget, const QIcon& icon, const QString& sLabel)
{
  qint32 iRet = QTabWidget::addTab(pWidget, icon, sLabel);
  InsertTabImpl(count()-1);
  return iRet;
}

//----------------------------------------------------------------------------------------
//
int CSlidingTabWidget::insertTab(int index, QWidget* pWidget, const QString& sLabel)
{
  qint32 iRet = QTabWidget::insertTab(index, pWidget, sLabel);
  InsertTabImpl(index);
  return iRet;
}

//----------------------------------------------------------------------------------------
//
int CSlidingTabWidget::insertTab(int index, QWidget* pWidget, const QIcon& icon, const QString& sLabel)
{
  qint32 iRet = QTabWidget::insertTab(index, pWidget, icon, sLabel);
  InsertTabImpl(index);
  return iRet;
}

//----------------------------------------------------------------------------------------
//
void CSlidingTabWidget::SlideInNext()
{
  SlideInNextImpl();
}

//----------------------------------------------------------------------------------------
//
void CSlidingTabWidget::SlideInPrev()
{
  SlideInPrevImpl();
}

//----------------------------------------------------------------------------------------
//
void CSlidingTabWidget::SlideInIdx(qint32 idx)
{
  SlideInIdxImpl(idx);
}

//----------------------------------------------------------------------------------------
//
qint32 CSlidingTabWidget::Count() const
{
  return count();
}

//----------------------------------------------------------------------------------------
//
qint32 CSlidingTabWidget::CurrentIndex() const
{
  return m_pStack->currentIndex();
}

//----------------------------------------------------------------------------------------
//
qint32 CSlidingTabWidget::IndexOf(QWidget* pWidget) const
{
  return indexOf(pWidget);
}

//----------------------------------------------------------------------------------------
//
QWidget* CSlidingTabWidget::Widget(qint32 idx) const
{
  return widget(idx);
}

//----------------------------------------------------------------------------------------
//
void CSlidingTabWidget::SetCurrentIndex(qint32 idx)
{
  // synch up all indices
  setCurrentIndex(idx);
  m_pStack->setCurrentIndex(idx);
  tabBar()->update();
}

//----------------------------------------------------------------------------------------
//
void CSlidingTabWidget::InsertTabImpl(qint32 /*iIndex*/)
{

}
