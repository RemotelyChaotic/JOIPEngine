#include "TimelineWidgetBackground.h"
#include "TimelineWidget.h"

#include <QLayout>
#include <QPainter>
#include <QPaintEvent>

CTimelineWidgetBackground::CTimelineWidgetBackground(QWidget* pParent) :
  QWidget{pParent}
{
  setMouseTracking(true);
}
CTimelineWidgetBackground::~CTimelineWidgetBackground() = default;

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetBackground::SetTimelineWidget(CTimelineWidget* pWidget)
{
  m_pWidget = pWidget;
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetBackground::paintEvent(QPaintEvent* pEvt)
{
  QWidget::paintEvent(pEvt);
}
