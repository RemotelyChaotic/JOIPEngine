#include "TimelineWidget.h"
#include "TimelineWidgetLayer.h"
#include "ui_TimelineWidget.h"

CTimelineWidget::CTimelineWidget(QWidget* pParent) :
  QScrollArea(pParent),
  m_spUi(std::make_unique<Ui::CTimelineWidget>())
{
  m_spUi->setupUi(this);
}

CTimelineWidget::~CTimelineWidget()
{
}

//----------------------------------------------------------------------------------------
//
QSize CTimelineWidget::minimumSizeHint() const
{
  return QSize(1,1);
}

//----------------------------------------------------------------------------------------
//
QSize CTimelineWidget::sizeHint() const
{
  return QScrollArea::sizeHint();
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::paintEvent(QPaintEvent* pEvent)
{

}
