#include "TimelineWidgetUtils.h"

#include <QTime>

void timeline::PaintUnusedAreaRect(QPainter* pPainter,
                                   qint32 iGridStartX, qint32 iAvailableWidth, qint32 iWidth, qint32 iHeight,
                                   qint64 iWindowStartMs, qint64 iMaximumSizeMs, qint64 iPageLengthMs,
                                   QColor outOfRangeCol)
{
  pPainter->save();
  //m_iPageLengthMs -                       iAvailableWidth
  //(m_iMaximumSizeMs - m_iWindowStartMs) - X
  double dEndLength = static_cast<double>((iMaximumSizeMs - iWindowStartMs) * iAvailableWidth) /
                   (0 == iPageLengthMs ? 1 : iPageLengthMs);
  if (dEndLength < static_cast<double>(iAvailableWidth))
  {
    qint32 iEndPos = static_cast<qint32>(std::round(dEndLength));
    pPainter->fillRect(QRect(iGridStartX + iEndPos, 0, iWidth - iEndPos, iHeight), outOfRangeCol);
  }
  pPainter->restore();
}

//----------------------------------------------------------------------------------------
//
namespace
{
  constexpr qint32 c_aPossibleGridTicks[] = {1, 5, 10, 50, 100, 500, 1000, 5000, 10'000,
                                             30'000, 60'000, 300'000, 600'000};
  constexpr qint32 c_aPossibleGridTicksLabels[] = {100, 100, 100, 100, 1000, 1000, 5'000, 10'000,
                                                   30'000, 60'000, 60'000, 60'000, 60'000};
  constexpr qint32 c_iIdealDistanceOfTicksMin = 100;
}

//----------------------------------------------------------------------------------------
//
void timeline::PaintTimestampGrid(QPainter* pPainter,
                                  qint32 iGridStartX, qint32 iAvailableWidth, qint32 iWidth, qint32 iHeight,
                                  qint64 iWindowStartMs, qint64, qint64 iPageLengthMs,
                                  QColor gridCol)
{
  pPainter->save();
  QPen pen(gridCol, 1);
  pPainter->setPen(pen);

  QFont font = pPainter->font();
  font.setPixelSize(9);
  QFontMetrics fm(font);
  pPainter->setFont(font);

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
    iFirstTick = (0 == iWindowStartMs % iGridToCheck) ?
                  iWindowStartMs : (iWindowStartMs / iGridToCheck * iGridToCheck);
    qint32 iFirstTickRelPos = iFirstTick - iWindowStartMs;
    dFirstTickX = static_cast<double>(iAvailableWidth * iFirstTickRelPos) /
                  (0 == iPageLengthMs ? 1 : iPageLengthMs);
    //qint32 iNrTicks = (m_iPageStep - iFirstTick) / iGridToCheck;
    dWidthBetweenTicks = static_cast<double>(iGridToCheck * iAvailableWidth) /
                         (0 == iPageLengthMs ? 1 : iPageLengthMs);

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
  while (dCounterX < static_cast<double>(iWidth) && 0 < dWidthBetweenTicks)
  {
    pPainter->drawLine(QLine(static_cast<qint32>(std::round(dCounterX)), iHeight,
                             static_cast<qint32>(std::round(dCounterX)), iHeight * 4 / 6));
    dCounterX += dWidthBetweenTicks;
  }
  const qint32 indexForMain = std::min(i+1, static_cast<qint32>(std::size(c_aPossibleGridTicks)) - 1);
  const qint32 iGridDist = c_aPossibleGridTicks[static_cast<size_t>(indexForMain)];
  const qint32 iLabelDist = c_aPossibleGridTicksLabels[static_cast<size_t>(indexForMain)];
  dCounterX = dFirstTickXMain + iGridStartX;
  qint32 iCounterTicks = iFirstTickMain;
  while (dCounterX < static_cast<double>(iWidth) && 0 < dWidthBetweenTicksMain)
  {
    const qint32 iXCoord = static_cast<qint32>(std::round(dCounterX));
    pPainter->drawLine(QLine(iXCoord, iHeight,
                             iXCoord, iHeight * 2 / 6));
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
      else if (r.x() + r.width() > iWidth)
      {
        r.setX(iWidth-r.width());
      }
      pPainter->drawText(r, Qt::AlignCenter, sLabel);
    }
    dCounterX += dWidthBetweenTicksMain;
    iCounterTicks += iGridDist;
  }

  pPainter->restore();
}

//----------------------------------------------------------------------------------------
//
qint64 timeline::PositionFromTime(qint32 iGridStartX, qint32 iAvailableWidth, qint32 iWidth,
                                  qint64 iWindowStartMs, qint64, qint64 iPageLengthMs,
                                  qint64 iInputTimeMs)
{
  if (iWindowStartMs > iInputTimeMs)
  {
    return -1;
  }
  else
  {
    const qint64 iPosRelMs = iInputTimeMs - iWindowStartMs;
    const double dPosMsRel = static_cast<double>(iAvailableWidth * iPosRelMs) / iPageLengthMs;
    qint32 iRes = iGridStartX + static_cast<qint32>(std::round(dPosMsRel));
    if (iRes > iWidth)
    {
      return -1;
    }
    return iRes;
  }
}
