#include "TimelineWidgetLayerBackground.h"

#include <QMouseEvent>
#include <QPainter>

CTimelineWidgetLayerBackground::CTimelineWidgetLayerBackground(QWidget* pParent) :
  QFrame{pParent},
  m_instructionInvalidMarker(":/resources/style/img/WarningIcon.png"),
  m_instructionMarker(":/resources/style/img/WarningIcon.png"),
  m_instructionMarkerOpenLeft(":/resources/style/img/WarningIcon.png"),
  m_instructionMarkerOpenRight(":/resources/style/img/WarningIcon.png")
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
void CTimelineWidgetLayerBackground::SetInstructionInvalidMarker(const QIcon& ico)
{
  m_instructionInvalidMarker = ico;
  repaint();
}

//----------------------------------------------------------------------------------------
//
const QIcon& CTimelineWidgetLayerBackground::InstructionInvalidMarker() const
{
  return m_instructionInvalidMarker;
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayerBackground::SetInstructionMarker(const QIcon& ico)
{
  m_instructionMarker = ico;
  repaint();
}

//----------------------------------------------------------------------------------------
//
const QIcon& CTimelineWidgetLayerBackground::InstructionMarker() const
{
  return m_instructionMarker;
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayerBackground::SetInstructionMarkerOpenLeft(const QIcon& ico)
{
  m_instructionMarkerOpenLeft = ico;
  repaint();
}

//----------------------------------------------------------------------------------------
//
const QIcon& CTimelineWidgetLayerBackground::InstructionMarkerOpenLeft() const
{
  return m_instructionMarkerOpenLeft;
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayerBackground::SetInstructionMarkerOpenRight(const QIcon& ico)
{
  m_instructionMarkerOpenRight = ico;
  repaint();
}

//----------------------------------------------------------------------------------------
//
const QIcon& CTimelineWidgetLayerBackground::InstructionMarkerOpenRight() const
{
  return m_instructionMarkerOpenRight;
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
namespace
{
  struct SPaintIconData
  {
    qint32 m_iPos;
    timeline::EInstructionVisualisationType m_type;
    qint32 m_iOpenType;
  };
  struct SPaintLineInstructionData
  {
    QLine m_line;
    qint32 m_iOpenType;
    bool m_bTerminated;
  };

  void PaintIcon(QPainter* pPainter,
                 const SPaintIconData& data,
                 QIcon& instructionInvalidMarker,
                 QIcon& instructionMarker,
                 QIcon& instructionMarkerOpenLeft,
                 QIcon& instructionMarkerOpenRight,
                 qint32 iWidth, qint32 iHeight)
  {
    using namespace timeline;
    switch (data.m_type)
    {
      case eNoInstructionType:
        instructionInvalidMarker.paint(pPainter, QRect(data.m_iPos-iWidth, -iHeight,
                                                       data.m_iPos+iWidth, iHeight));
        break;
      case eOpening:
        instructionMarker.paint(pPainter, QRect(data.m_iPos-iWidth, -iHeight,
                                                data.m_iPos+iWidth, iHeight));
        break;
      case eClosing:
        instructionMarkerOpenLeft.paint(pPainter, QRect(data.m_iPos-iWidth, -iHeight,
                                                        data.m_iPos+iWidth, iHeight));
        break;
      case eSingle:
        instructionMarkerOpenRight.paint(pPainter, QRect(data.m_iPos-iWidth, -iHeight,
                                                         data.m_iPos+iWidth, iHeight));
        break;
    }
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

  // draw instructions
  if (nullptr != m_spLayer)
  {
    using namespace timeline;
    std::vector<SPaintIconData> vPaintIcons;
    std::vector<SPaintLineInstructionData> vPaintLineInstr;
    for (const auto& [time, instr] : m_spLayer->m_vspInstructions)
    {
      if (time >= m_iWindowStartMs && m_iWindowStartMs + m_iPageLengthMs > time)
      {
        qint32 iPos = timeline::PositionFromTime(0, width(), width(), m_iWindowStartMs,
                                                 m_iMaximumSizeMs, m_iPageLengthMs,
                                                 time);
        auto type = timeline::InstructionVisualisationType(instr->m_sInstructionType);

        vPaintIcons.push_back({iPos, type.first, type.second});

        if (EInstructionVisualisationType::eOpening == type.first)
        {
          vPaintLineInstr.push_back({QLine(iPos, height() / 2, width(), height() / 2), type.second, false});
        }
        else if (EInstructionVisualisationType::eClosing == type.first)
        {
          for (auto it = vPaintLineInstr.rbegin(); vPaintLineInstr.rend() != it; --it)
          {
            if (!it->m_bTerminated)
            {
              if (it->m_iOpenType & type.second)
              {
                it->m_line.setPoints(it->m_line.p1(), QPoint(iPos, it->m_line.p1().y()));
                it->m_bTerminated = true;
              }
            }
          }
        }
      }
    }

    // log2 is too slow, so we do a lookup
    static const std::map<qint32,qint32> c_viOffsetLookup = {
      {1, 1}, {2, 2}, {4, 3}, {8, 4}, {16, 5}
    };
    constexpr qint32 c_iYOffsetFactor = 10;
    constexpr qint32 c_iIconWidth = 10;
    constexpr qint32 c_iIconHeight = 10;

    {
      painter.save();
      QPen pen(Qt::red);
      painter.setPen(pen);
      for (const auto& lineData : vPaintLineInstr)
      {
        qint32 iPower = c_viOffsetLookup.find(lineData.m_iOpenType)->second;
        qint32 iOffset = -1*iPower%2 * iPower/2 * c_iYOffsetFactor;
        painter.drawLine(lineData.m_line.x1(), lineData.m_line.y1() + iOffset,
                         lineData.m_line.x2(), lineData.m_line.y2() + iOffset);
      }
      painter.restore();
    }

    painter.save();
    painter.translate(QPoint{0, height() / 2});
    for (const auto& iconData : vPaintIcons)
    {
      if (-1 == iconData.m_iOpenType)
      {
        PaintIcon(&painter, iconData, m_instructionInvalidMarker, m_instructionMarker,
                  m_instructionMarkerOpenLeft, m_instructionMarkerOpenRight,
                  c_iIconWidth, c_iIconHeight);
      }
      else
      {
        for (const auto& [flagVal, index] : c_viOffsetLookup)
        {
          if (iconData.m_iOpenType & flagVal)
          {
            qint32 iOffset = -1*index%2 * index/2 * c_iYOffsetFactor;
            painter.save();
            painter.translate(QPoint{0, iOffset});
            PaintIcon(&painter, iconData, m_instructionInvalidMarker, m_instructionMarker,
                      m_instructionMarkerOpenLeft, m_instructionMarkerOpenRight,
                      c_iIconWidth, c_iIconHeight);
            painter.restore();
          }
        }
      }
    }
    painter.restore();
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayerBackground::mouseReleaseEvent(QMouseEvent* pEvt)
{
  if (nullptr != pEvt)
  {
    if (Qt::RightButton == pEvt->button())
    {
      emit SignalOpenInsertContextMenuAt(pEvt->globalPos());
    }
  }
}
