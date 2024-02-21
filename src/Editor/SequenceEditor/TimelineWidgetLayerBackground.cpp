#include "TimelineWidgetLayerBackground.h"

#include <QMouseEvent>
#include <QPainter>
#include <QRadialGradient>

CTimelineWidgetLayerBackground::CTimelineWidgetLayerBackground(QWidget* pParent) :
  QFrame{pParent},
  m_instructionInvalidMarker(":/resources/style/img/WarningIcon.png"),
  m_instructionMarker(":/resources/style/img/WarningIcon.png"),
  m_instructionMarkerOpenLeft(":/resources/style/img/WarningIcon.png"),
  m_instructionMarkerOpenRight(":/resources/style/img/WarningIcon.png"),
  m_instructionConnectorColor(Qt::red),
  m_selectionColor(Qt::white)
{
  setMouseTracking(true);
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
void CTimelineWidgetLayerBackground::SetIconWidth(qint32 iWidth)
{
  if (m_iIconWidth != iWidth)
  {
    m_iIconWidth = iWidth;
    repaint();
  }
}

//----------------------------------------------------------------------------------------
//
qint32 CTimelineWidgetLayerBackground::IconWidth() const
{
  return m_iIconWidth;
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayerBackground::SetIconHeight(qint32 iHeight)
{
  if (m_iIconHeight != iHeight)
  {
    m_iIconHeight = iHeight;
    repaint();
  }
}

//----------------------------------------------------------------------------------------
//
qint32 CTimelineWidgetLayerBackground::IconHeight() const
{
  return m_iIconHeight;
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayerBackground::SetInstructionConnectorColor(const QColor& col)
{
  m_instructionConnectorColor = col;
  repaint();
}

//----------------------------------------------------------------------------------------
//
const QColor& CTimelineWidgetLayerBackground::InstructionConnectorColor() const
{
  return m_instructionConnectorColor;
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
void CTimelineWidgetLayerBackground::SetSelectionColor(const QColor& col)
{
  m_selectionColor = col;
}

//----------------------------------------------------------------------------------------
//
const QColor& CTimelineWidgetLayerBackground::SelectionColor() const
{
  return m_selectionColor;
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
void CTimelineWidgetLayerBackground::ClearSelection()
{
  m_iSelectedInstr = -1;
  m_iHighlightedInstr = -1;
  repaint();
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<SSequenceInstruction> CTimelineWidgetLayerBackground::InstructionFromTime(qint64 iTime) const
{
  auto it = std::find_if(m_spLayer->m_vspInstructions.begin(), m_spLayer->m_vspInstructions.end(),
                         [&iTime](const sequence::tTimedInstruction& pair) {
                           return pair.first == iTime;
                         });
  if (m_spLayer->m_vspInstructions.end() != it)
  {
    return it->second;
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
qint64 CTimelineWidgetLayerBackground::CurrentlySelectedInstructionTime() const
{
  return m_iSelectedInstr;
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<SSequenceInstruction> CTimelineWidgetLayerBackground::CurrentlySelectedInstruction() const
{
  return InstructionFromTime(m_iSelectedInstr);
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
void CTimelineWidgetLayerBackground::SetSelectedInstruction(qint64 iIdx)
{
  if (m_iSelectedInstr != iIdx && -1 != iIdx && nullptr != m_spLayer)
  {
    m_iSelectedInstr = -1;
    if (nullptr == InstructionFromTime(iIdx))
    {
      m_iSelectedInstr = iIdx;
      emit SignalEditInstruction(m_iSelectedInstr);
    }
  }
  else
  {
    m_iSelectedInstr = -1;
  }
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetLayerBackground::SetTimeGrid(qint64 iGrid)
{
  m_iTimeGrid = iGrid;
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
    qint64 m_iTime;
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
                 QColor selectionColor, bool bSelected, bool bHighlighted,
                 qint32 iWidth, qint32 iHeight)
  {
    if (bSelected || bHighlighted)
    {
      QRadialGradient radialGrad(QPoint(data.m_iPos, 0), iWidth/2+5);
      radialGrad.setColorAt(0.000, selectionColor);
      QColor col2 = selectionColor;
      col2.setAlpha(0.0);
      radialGrad.setColorAt(1.000, col2);

      QPen pen;
      pen.setWidth(iWidth+10);
      pen.setColor(selectionColor);
      pen.setBrush(radialGrad);

      pPainter->setPen(pen);
      pPainter->drawPoint(data.m_iPos, 0);
    }

    using namespace timeline;
    switch (data.m_type)
    {
      case eNoInstructionType:
        instructionInvalidMarker.paint(pPainter, QRect(data.m_iPos-iWidth/2, -iHeight/2,
                                                       iWidth, iHeight));
        break;
      case eOpening:
        instructionMarker.paint(pPainter, QRect(data.m_iPos-iWidth/2, -iHeight/2,
                                                iWidth, iHeight));
        break;
      case eClosing:
        instructionMarkerOpenLeft.paint(pPainter, QRect(data.m_iPos-iWidth/2, -iHeight/2,
                                                        iWidth, iHeight));
        break;
      case eSingle:
        instructionMarkerOpenRight.paint(pPainter, QRect(data.m_iPos-iWidth/2, -iHeight/2,
                                                         iWidth, iHeight));
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

        vPaintIcons.push_back({iPos, type.first, type.second, time});

        if (EInstructionVisualisationType::eOpening == type.first)
        {
          vPaintLineInstr.push_back({QLine(iPos, height() / 2, width(), height() / 2), type.second, false});
        }
        else if (EInstructionVisualisationType::eClosing == type.first)
        {
          bool bTerminatedAny = false;
          for (auto it = vPaintLineInstr.rbegin(); vPaintLineInstr.rend() != it; ++it)
          {
            if (!it->m_bTerminated)
            {
              if (it->m_iOpenType & type.second)
              {
                it->m_line.setPoints(it->m_line.p1(), QPoint(iPos, it->m_line.p1().y()));
                it->m_bTerminated = true;
                bTerminatedAny = true;
              }
            }
          }
          if (!bTerminatedAny)
          {
            vPaintLineInstr.push_back({QLine(0, height() / 2, iPos, height() / 2), type.second, true});
          }
        }
      }
    }

    // log2 is too slow, so we do a lookup
    static const std::map<qint32,qint32> c_viOffsetLookup = {
      {1, 1}, {2, 2}, {4, 3}, {8, 4}, {16, 5}
    };
    constexpr qint32 c_iYOffsetFactor = 10;

    {
      painter.save();
      QPen pen(m_instructionConnectorColor);
      painter.setPen(pen);
      for (const auto& lineData : vPaintLineInstr)
      {
        qint32 iPower = c_viOffsetLookup.find(lineData.m_iOpenType)->second;
        qint32 iOffset = (iPower%2 == 0 ? 1 : -1) * iPower/2 * c_iYOffsetFactor;
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
                  m_selectionColor,
                  iconData.m_iTime == m_iSelectedInstr, iconData.m_iTime == m_iHighlightedInstr,
                  m_iIconWidth, m_iIconHeight);
      }
      else
      {
        for (const auto& [flagVal, index] : c_viOffsetLookup)
        {
          if (iconData.m_iOpenType & flagVal)
          {
            qint32 iOffset = (index%2 == 0 ? 1 : -1) * index/2 * c_iYOffsetFactor;
            painter.save();
            painter.translate(QPoint{0, iOffset});
            PaintIcon(&painter, iconData, m_instructionInvalidMarker, m_instructionMarker,
                      m_instructionMarkerOpenLeft, m_instructionMarkerOpenRight,
                      m_selectionColor,
                      iconData.m_iTime == m_iSelectedInstr, iconData.m_iTime == m_iHighlightedInstr,
                      m_iIconWidth, m_iIconHeight);
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
void CTimelineWidgetLayerBackground::mouseMoveEvent(QMouseEvent* pEvt)
{
  if (nullptr != pEvt)
  {
    auto range = GetTimeRangeAroundCursor(pEvt->pos());
    qint64 iHighlight = MostLikelyInstruction(range);
    if (iHighlight != m_iHighlightedInstr)
    {
      m_iHighlightedInstr = iHighlight;
      repaint();
    }
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
      qint64 iCursorTime =
          timeline::GetTimeFromCursorPos(pEvt->pos().x(), 0, width(),
                                         m_iWindowStartMs, m_iMaximumSizeMs, m_iPageLengthMs);
      iCursorTime = ConstrainToGrid(iCursorTime);
      if (-1 != iCursorTime && nullptr != m_spLayer)
      {
        if (nullptr == InstructionFromTime(iCursorTime))
        {
          emit SignalOpenInsertContextMenuAt(pEvt->globalPos(), iCursorTime);
        }
      }
    }
    else if (Qt::LeftButton == pEvt->button())
    {
      auto range = GetTimeRangeAroundCursor(pEvt->pos());
      m_iSelectedInstr = MostLikelyInstruction(range);
      repaint();

      if (-1 != m_iSelectedInstr && nullptr != m_spLayer)
      {
        if (nullptr != InstructionFromTime(m_iSelectedInstr))
        {
          emit SignalEditInstruction(m_iSelectedInstr);
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
qint64 CTimelineWidgetLayerBackground::ConstrainToGrid(qint64 iValue)
{
  qint64 iGriddedconstrainedPos = iValue;
  if (0 < m_iTimeGrid)
  {
    double dVal = std::round(static_cast<double>(iValue) / m_iTimeGrid) * m_iTimeGrid;
    iGriddedconstrainedPos = static_cast<qint64>(dVal);
  }
  return iGriddedconstrainedPos;
}

//----------------------------------------------------------------------------------------
//
std::tuple<qint64, qint64, qint64> CTimelineWidgetLayerBackground::GetTimeRangeAroundCursor(QPoint p) const
{
  qint32 iCursorPos = p.x();
  qint32 iCursorPosMin = std::max(iCursorPos - 10, 0);
  qint32 iCursorPosMax = std::min(iCursorPos + 10, width());
  qint64 iCursorTimeMin =
      timeline::GetTimeFromCursorPos(iCursorPosMin, 0, width(),
                                     m_iWindowStartMs, m_iMaximumSizeMs, m_iPageLengthMs);
  qint64 iCursorTime =
      timeline::GetTimeFromCursorPos(iCursorPos, 0, width(),
                                     m_iWindowStartMs, m_iMaximumSizeMs, m_iPageLengthMs);
  qint64 iCursorTimeMax =
      timeline::GetTimeFromCursorPos(iCursorPosMax, 0, width(),
                                     m_iWindowStartMs, m_iMaximumSizeMs, m_iPageLengthMs);
  return {iCursorTimeMin, iCursorTime, iCursorTimeMax};
}

//----------------------------------------------------------------------------------------
//
qint64 CTimelineWidgetLayerBackground::MostLikelyInstruction(const std::tuple<qint64, qint64, qint64>& timeRange) const
{
  qint64 iClickPos = std::get<1>(timeRange);
  qint64 iMostLikelyHit = -1;
  qint64 iLastDist = INT_MAX;
  for (const auto& [time, _] : m_spLayer->m_vspInstructions)
  {
    if (time < std::get<0>(timeRange) || time > std::get<2>(timeRange))
    {
      continue;
    }
    qint64 iDist = std::abs(time - iClickPos);
    if (iDist < iLastDist)
    {
      iLastDist = iDist;
      iMostLikelyHit = time;
    }
  }
  return iMostLikelyHit;
}
