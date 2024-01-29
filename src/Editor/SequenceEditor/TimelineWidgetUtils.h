#ifndef CTIMELINEWIDGETUTILS_H
#define CTIMELINEWIDGETUTILS_H

#include <QPainter>
#include <QtGlobal>

namespace timeline
{
  constexpr qint64 c_iDefaultStepSize = 10'000;
  constexpr qint64 c_iDefaultLength = 15'000;

  void PaintUnusedAreaRect(QPainter* pPainter,
                           qint32 iGridStartX, qint32 iAvailableWidth, qint32 iWidth, qint32 iHeight,
                           qint64 iWindowStartMs, qint64 iMaximumSizeMs, qint64 iPageLengthMs,
                           QColor outOfRangeCol);

  void PaintTimestampGrid(QPainter* pPainter,
                          qint32 iGridStartX, qint32 iAvailableWidth, qint32 iWidth, qint32 iHeight,
                          qint64 iWindowStartMs, qint64 iMaximumSizeMs, qint64 iPageLengthMs,
                          QColor gridCol);

  qint64 PositionFromTime(qint32 iGridStartX, qint32 iAvailableWidth, qint32 iWidth,
                          qint64 iWindowStartMs, qint64 iMaximumSizeMs, qint64 iPageLengthMs,
                          qint64 iInputTimeMs);
}

#endif // CTIMELINEWIDGETUTILS_H
