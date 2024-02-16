#include "TimelineWidgetControls.h"

#include "Widgets/ZoomComboBox.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QPainter>
#include <QPaintEvent>
#include <QTime>

CTimelineWidgetControls::CTimelineWidgetControls(QWidget* pParent) :
  QFrame(pParent)
{
  QHBoxLayout* pLayout = new QHBoxLayout(this);
  pLayout->setContentsMargins(0, 0, 0, 0);
  pLayout->setSpacing(0);

  m_pControls = new QWidget(this);
  m_pControls->setObjectName("Controls");
  QHBoxLayout* pLayoutControls = new QHBoxLayout(m_pControls);
  pLayoutControls->setContentsMargins(0, 0, 0, 0);
  pLayoutControls->setSpacing(0);

  m_pComboZoom = new CZoomComboBox(m_pControls);
  m_pComboZoom->SetSteps({25, 50, 100, 150, 200, 500, 1000});
  connect(m_pComboZoom, &CZoomComboBox::SignalZoomChanged, this,
          &CTimelineWidgetControls::SlotZoomChanged);
  pLayoutControls->addWidget(m_pComboZoom);

  m_pCurrentLabel = new QLabel(m_pControls);
  pLayoutControls->addWidget(m_pCurrentLabel);

  pLayoutControls->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));

  pLayout->addWidget(m_pControls);
  pLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));

  SetHeaderSize(m_pControls->size() + QSize(50, 0));
  setMouseTracking(true);
}
CTimelineWidgetControls::~CTimelineWidgetControls() = default;

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetControls::SetCursorColor(const QColor& col)
{
  m_cursorCol = col;
  repaint();
}

//----------------------------------------------------------------------------------------
//
const QColor& CTimelineWidgetControls::CursorColor() const
{
  return m_cursorCol;
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetControls::SetGridColor(const QColor& col)
{
  m_gridCol = col;
  repaint();
}

//----------------------------------------------------------------------------------------
//
const QColor& CTimelineWidgetControls::GridColor() const
{
  return m_gridCol;
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetControls::SetOutOfRangeColor(const QColor& col)
{
  m_outOfRangeCol = col;
  repaint();
}

//----------------------------------------------------------------------------------------
//
const QColor& CTimelineWidgetControls::OutOfRangeColor() const
{
  return m_outOfRangeCol;
}

//----------------------------------------------------------------------------------------
//
qint64 CTimelineWidgetControls::CurrentTimeStamp() const
{
  return m_iSelectedTime;
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetControls::SetCurrentCursorPos(qint32 iX)
{
  if (m_iCursorPos != iX)
  {
    m_iCursorPos = iX;
    UpdateCurrentLabel();
    repaint();
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetControls::SetCurrentTimeStamp(qint64 iTimeMs)
{
  if (m_iSelectedTime != iTimeMs)
  {
    m_iSelectedTime = iTimeMs;
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetControls::SetCurrentWindow(qint64 iStartMs, qint64 iPageLengthMs)
{
  if (m_iWindowStartMs != iStartMs || m_iPageLengthMs != iPageLengthMs)
  {
    m_iWindowStartMs = iStartMs;
    m_iPageLengthMs = iPageLengthMs;
    UpdateCurrentLabel();
    repaint();
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetControls::SetHeaderSize(QSize s)
{
  m_pControls->setFixedWidth(s.width());
  UpdateCurrentLabel();
  repaint();
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetControls::SetTimeMaximum(qint64 iTimeMs)
{
  if (iTimeMs != m_iMaximumSizeMs)
  {
    m_iMaximumSizeMs = iTimeMs;
    UpdateCurrentLabel();
    repaint();
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetControls::ZoomIn()
{
  if (m_pComboZoom->currentIndex() < m_pComboZoom->count() - 1)
  {
    m_pComboZoom->setCurrentIndex(m_pComboZoom->currentIndex()+1);
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetControls::ZoomOut()
{
  if (m_pComboZoom->currentIndex() > 0)
  {
    m_pComboZoom->setCurrentIndex(m_pComboZoom->currentIndex()-1);
  }
}

//----------------------------------------------------------------------------------------
//
qint32 CTimelineWidgetControls::CursorFromCurrentTime() const
{
  const qint32 iGridStartX = m_pControls->width();
  const qint32 iAvailableWidth = width() - iGridStartX;

  return timeline::PositionFromTime(iGridStartX, iAvailableWidth, width(), m_iWindowStartMs,
                                    m_iMaximumSizeMs, m_iPageLengthMs, m_iSelectedTime);
}

//----------------------------------------------------------------------------------------
//
qint32 CTimelineWidgetControls::CurrentCursorPos() const
{
  return m_iCursorPos;
}

//----------------------------------------------------------------------------------------
//
qint64 CTimelineWidgetControls::TimeFromCursor() const
{
  return m_iCursorTime;
}

//----------------------------------------------------------------------------------------
//
qint32 CTimelineWidgetControls::Zoom() const
{
  return m_pComboZoom->Zoom();
}

//----------------------------------------------------------------------------------------
//
QSize CTimelineWidgetControls::sizeHint() const
{
  return m_pComboZoom->sizeHint();
}

//----------------------------------------------------------------------------------------
//
QSize CTimelineWidgetControls::minimumSizeHint() const
{
  return m_pComboZoom->minimumSizeHint();
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetControls::mouseMoveEvent(QMouseEvent* pEvent)
{
  if (nullptr != pEvent)
  {
    SetCurrentCursorPos(pEvent->pos().x());
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetControls::paintEvent(QPaintEvent* pEvt)
{
  QWidget::paintEvent(pEvt);
  QPainter painter(this);

  const qint32 iGridStartX = m_pControls->width();
  const qint32 iAvailableWidth = width() - iGridStartX;

  // draw rect over unused area
  timeline::PaintUnusedAreaRect(&painter, iGridStartX, iAvailableWidth, width(), height(),
                                m_iWindowStartMs, m_iMaximumSizeMs, m_iPageLengthMs, m_outOfRangeCol);

  // draw timestamp grid
  timeline::PaintTimestampGrid(&painter, iGridStartX, iAvailableWidth, width(), height(),
                               m_iWindowStartMs, m_iMaximumSizeMs, m_iPageLengthMs, m_gridCol);

  // draw current mouse indicator
  if (m_iCursorPos >= m_pControls->width() && m_iCursorPos <= width())
  {
    painter.save();
    QPen pen(m_cursorCol, 2);
    painter.setPen(pen);
    painter.drawLine(QLine(m_iCursorPos, 0, m_iCursorPos, height()));
    painter.restore();
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetControls::SlotZoomChanged(qint32 iZoom)
{
  UpdateCurrentLabel();
  emit SignalZoomChanged(iZoom);
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetControls::UpdateCurrentLabel()
{
  const qint32 iGridStartX = m_pControls->width();
  const qint32 iAvailableWidth = width() - iGridStartX;

  m_iCursorTime = timeline::GetTimeFromCursorPos(m_iCursorPos, iGridStartX, iAvailableWidth,
                                                 m_iWindowStartMs, m_iMaximumSizeMs, m_iPageLengthMs);

  if (0 > m_iCursorTime)
  {
    m_pCurrentLabel->setText("00:00.000");
  }
  else
  {
    m_pCurrentLabel->setText(QTime(0,0)
                                 .addMSecs(m_iCursorTime)
                                 .toString("mm:ss.zzz"));
  }
}
