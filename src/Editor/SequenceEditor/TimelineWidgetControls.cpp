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
  pLayout->addWidget(m_pControls);
  pLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));

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

  m_pControls->setFixedWidth(m_pComboZoom->sizeHint().width() + m_pCurrentLabel->sizeHint().width());

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
  {
    painter.save();
    //m_iPageLengthMs -                       iAvailableWidth
    //(m_iMaximumSizeMs - m_iWindowStartMs) - X
    double dEndLength = static_cast<double>((m_iMaximumSizeMs - m_iWindowStartMs) * iAvailableWidth) /
                     (0 == m_iPageLengthMs ? 1 : m_iPageLengthMs);
    if (dEndLength < static_cast<double>(iAvailableWidth))
    {
      qint32 iEndPos = static_cast<qint32>(std::round(dEndLength));
      painter.fillRect(QRect(iGridStartX + iEndPos, 0, width() - iEndPos, height()), m_outOfRangeCol);
    }
    painter.restore();
  }

  // draw timestamp grid
  {
    painter.save();
    QPen pen(m_gridCol, 1);
    painter.setPen(pen);

    QFont font = painter.font();
    font.setPixelSize(9);
    QFontMetrics fm(font);
    painter.setFont(font);

    constexpr qint32 c_aPossibleGridTicks[] = {1, 5, 10, 50, 100, 500, 1000, 5000, 10'000,
                                               30'000, 60'000, 300'000, 600'000};
    constexpr qint32 c_aPossibleGridTicksLabels[] = {100, 100, 100, 100, 1000, 1000, 5'000, 10'000,
                                                     30'000, 60'000, 60'000, 60'000, 60'000};
    constexpr qint32 c_iIdealDistanceOfTicksMin = 100;

    bool bBreakAtNext = false;
    qint32 iFirstTickMain = 0;
    double dFirstTickXMain = 0.0;
    double dWidthBetweenTicksMain = c_iIdealDistanceOfTicksMin;
    qint32 iFirstTick = 0;
    double dFirstTickX = 0.0;
    double dWidthBetweenTicks = c_iIdealDistanceOfTicksMin;
    qint32 i = static_cast<qint32>(std::size(c_aPossibleGridTicks)) - 1;
    for (; -1 < i; --i)
    {
      iFirstTickMain = iFirstTick;
      dFirstTickXMain = dFirstTickX;
      dWidthBetweenTicksMain = dWidthBetweenTicks;

      qint32 iGridToCheck = c_aPossibleGridTicks[static_cast<size_t>(i)];
      iFirstTick = (0 == m_iWindowStartMs % iGridToCheck) ?
                    m_iWindowStartMs : (m_iWindowStartMs / iGridToCheck * iGridToCheck);
      qint32 iFirstTickRelPos = iFirstTick - m_iWindowStartMs;
      dFirstTickX = static_cast<double>(iAvailableWidth * iFirstTickRelPos) /
                    (0 == m_iPageLengthMs ? 1 : m_iPageLengthMs);
      //qint32 iNrTicks = (m_iPageStep - iFirstTick) / iGridToCheck;
      dWidthBetweenTicks = static_cast<double>(iGridToCheck * iAvailableWidth) /
                           (0 == m_iPageLengthMs ? 1 : m_iPageLengthMs);

      if (bBreakAtNext)
      {
        break;
      }

      if (c_iIdealDistanceOfTicksMin-70 < dWidthBetweenTicks &&
          c_iIdealDistanceOfTicksMin+50 > dWidthBetweenTicks)
      {
        bBreakAtNext = true;
      }
    }

    double dCounterX = dFirstTickX + iGridStartX;
    while (dCounterX < static_cast<double>(width()) && 0 < dWidthBetweenTicks)
    {
      painter.drawLine(QLine(static_cast<qint32>(std::round(dCounterX)), height(),
                             static_cast<qint32>(std::round(dCounterX)), height() * 4 / 6));
      dCounterX += dWidthBetweenTicks;
    }
    const qint32 indexForMain = std::min(i+1, static_cast<qint32>(std::size(c_aPossibleGridTicks)) - 1);
    const qint32 iGridDist = c_aPossibleGridTicks[static_cast<size_t>(indexForMain)];
    const qint32 iLabelDist = c_aPossibleGridTicksLabels[static_cast<size_t>(indexForMain)];
    dCounterX = dFirstTickXMain + iGridStartX;
    qint32 iCounterTicks = iFirstTickMain;
    while (dCounterX < static_cast<double>(width()) && 0 < dWidthBetweenTicksMain)
    {
      const qint32 iXCoord = static_cast<qint32>(std::round(dCounterX));
      painter.drawLine(QLine(iXCoord, height(),
                             iXCoord, height() * 2 / 6));
      if (0 == iCounterTicks % iLabelDist)
      {
        QTime t(0, 0);
        t = t.addMSecs(iCounterTicks);
        const QString sLabel = t.toString(iLabelDist < 1000 ? "ss.zzz" : "mm:ss");
        QRect r = fm.tightBoundingRect(sLabel);
        r.translate(iXCoord - r.width() / 2, r.height() + 1);
        r.adjust(-1, -1, 1, 1);
        if (r.x() < iGridStartX)
        {
          r.setX(iGridStartX);
        }
        else if (r.x() + r.width() > width())
        {
          r.setX(width()-r.width());
        }
        painter.drawText(r, Qt::AlignCenter, sLabel);
      }
      dCounterX += dWidthBetweenTicksMain;
      iCounterTicks += iGridDist;
    }

    painter.restore();
  }

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

  if (m_iCursorPos < iGridStartX)
  {
    m_pCurrentLabel->setText("00:00.000");
  }
  else
  {
    const qint32 iPosXRel = m_iCursorPos - iGridStartX;
    const double dPosMsRel = static_cast<double>(m_iPageLengthMs * iPosXRel) / iAvailableWidth;
    m_pCurrentLabel->setText(QTime(0,0)
                                 .addMSecs(m_iWindowStartMs + static_cast<qint32>(std::round(dPosMsRel)))
                                 .toString("mm:ss.zzz"));
  }
}
