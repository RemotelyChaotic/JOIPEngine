#include "TimelineWidgetLayerBackground.h"

CTimelineWidgetLayerBackground::CTimelineWidgetLayerBackground(QWidget* pParent) :
  QFrame{pParent}
{
}
CTimelineWidgetLayerBackground::~CTimelineWidgetLayerBackground() = default;

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayerBackground::SetGridColor(const QColor& col)
{
  m_gridCol = col;
  repaint();
}

//----------------------------------------------------------------------------------------
//
const QColor& CTimelineWidgetLayerBackground::GridColor() const
{
  return m_gridCol;
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayerBackground::SetOutOfRangeColor(const QColor& col)
{
  m_outOfRangeCol = col;
  repaint();
}

//----------------------------------------------------------------------------------------
//
const QColor& CTimelineWidgetLayerBackground::OutOfRangeColor() const
{
  return m_outOfRangeCol;
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayerBackground::SetTimelineBackgroundColor(const QColor& col)
{
  m_timelineBgColor = col;
  setStyleSheet("background-color:" + col.name());
}

//----------------------------------------------------------------------------------------
//
const QColor& CTimelineWidgetLayerBackground::TimelineBackgroundColor() const
{
  return m_timelineBgColor;
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayerBackground::SetLayer(const tspSequenceLayer& spLayer)
{
  m_spLayer = spLayer;
  repaint();
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayerBackground::SetCurrentWindow(qint64 iStartMs, qint64 iPageLengthMs)
{
  if (m_iWindowStartMs != iStartMs || m_iPageLengthMs != iPageLengthMs)
  {
    m_iWindowStartMs = iStartMs;
    m_iPageLengthMs = iPageLengthMs;
    repaint();
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayerBackground::SetTimeMaximum(qint64 iTimeMs)
{
  if (iTimeMs != m_iMaximumSizeMs)
  {
    m_iMaximumSizeMs = iTimeMs;
    repaint();
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayerBackground::paintEvent(QPaintEvent* pEvt)
{
  QFrame::paintEvent(pEvt);

  QPainter painter(this);

  // draw rect over unused area
  timeline::PaintUnusedAreaRect(&painter, 0, width(), width(), height(),
                                m_iWindowStartMs, m_iMaximumSizeMs, m_iPageLengthMs, m_outOfRangeCol);

  // draw timestamp grid
  timeline::PaintTimestampGrid(&painter, 0, width(), width(), height(),
                               m_iWindowStartMs, m_iMaximumSizeMs, m_iPageLengthMs, m_gridCol);

  if (nullptr != m_spLayer)
  {
    for (const auto& [time, _] : m_spLayer->m_vspInstructions)
    {
      if (time >= m_iWindowStartMs && m_iWindowStartMs + m_iPageLengthMs > time)
      {
        qint32 iPos = timeline::PositionFromTime(0, width(), width(), m_iWindowStartMs,
                                                 m_iMaximumSizeMs, m_iPageLengthMs,
                                                 time);
        Q_UNUSED(iPos)
      }
    }
  }
}
