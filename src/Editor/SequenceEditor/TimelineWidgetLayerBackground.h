#ifndef CTIMELINEWIDGETLAYERBACKGROUND_H
#define CTIMELINEWIDGETLAYERBACKGROUND_H

#include "TimelineWidgetUtils.h"

#include "Systems/Sequence/Sequence.h"

#include <QFrame>

class CTimelineWidgetLayerBackground : public QFrame
{
public:
  explicit CTimelineWidgetLayerBackground(QWidget* pParent = nullptr);
  ~CTimelineWidgetLayerBackground() override;

  void SetGridColor(const QColor& col);
  const QColor& GridColor() const;
  void SetOutOfRangeColor(const QColor& col);
  const QColor& OutOfRangeColor() const;
  void SetTimelineBackgroundColor(const QColor& col);
  const QColor& TimelineBackgroundColor() const;

  void SetLayer(const tspSequenceLayer& spLayer);

  void SetCurrentWindow(qint64 iStartMs, qint64 iPageLengthMs);
  void SetTimeMaximum(qint64 iTimeMs);

protected:
  void paintEvent(QPaintEvent* pEvt) override;

private:
  tspSequenceLayer               m_spLayer;
  QColor                         m_outOfRangeCol;
  QColor                         m_gridCol;
  QColor                         m_timelineBgColor;
  qint64                         m_iMaximumSizeMs = timeline::c_iDefaultLength;
  qint64                         m_iPageLengthMs = timeline::c_iDefaultStepSize;
  qint64                         m_iWindowStartMs = 0;
};

#endif // CTIMELINEWIDGETLAYERBACKGROUND_H
