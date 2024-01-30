#ifndef CTIMELINEWIDGETLAYERBACKGROUND_H
#define CTIMELINEWIDGETLAYERBACKGROUND_H

#include "TimelineWidgetUtils.h"

#include "Systems/Sequence/Sequence.h"

#include <QFrame>
#include <QIcon>

class CTimelineWidgetLayerBackground : public QFrame
{
  Q_OBJECT
  Q_PROPERTY(QIcon instructionInvalidMarker READ InstructionInvalidMarker WRITE SetInstructionInvalidMarker)
  Q_PROPERTY(QIcon instructionMarker READ InstructionMarker WRITE SetInstructionMarker)
  Q_PROPERTY(QIcon instructionMarkerOpenLeft READ InstructionMarkerOpenLeft WRITE SetInstructionMarkerOpenLeft)
  Q_PROPERTY(QIcon instructionMarkerOpenRight READ InstructionMarkerOpenRight WRITE SetInstructionMarkerOpenRight)

public:
  explicit CTimelineWidgetLayerBackground(QWidget* pParent = nullptr);
  ~CTimelineWidgetLayerBackground() override;

  void SetGridColor(const QColor& col);
  const QColor& GridColor() const;
  void SetInstructionInvalidMarker(const QIcon& ico);
  const QIcon& InstructionInvalidMarker() const;
  void SetInstructionMarker(const QIcon& ico);
  const QIcon& InstructionMarker() const;
  void SetInstructionMarkerOpenLeft(const QIcon& ico);
  const QIcon& InstructionMarkerOpenLeft() const;
  void SetInstructionMarkerOpenRight(const QIcon& ico);
  const QIcon& InstructionMarkerOpenRight() const;
  void SetOutOfRangeColor(const QColor& col);
  const QColor& OutOfRangeColor() const;
  void SetTimelineBackgroundColor(const QColor& col);
  const QColor& TimelineBackgroundColor() const;

  void SetLayer(const tspSequenceLayer& spLayer);

  void SetCurrentWindow(qint64 iStartMs, qint64 iPageLengthMs);
  void SetTimeMaximum(qint64 iTimeMs);

signals:
  void SignalOpenInsertContextMenuAt(QPoint p);

protected:
  void paintEvent(QPaintEvent* pEvt) override;
  void mouseReleaseEvent(QMouseEvent* pEvt) override;

private:
  tspSequenceLayer               m_spLayer;
  QIcon                          m_instructionInvalidMarker;
  QIcon                          m_instructionMarker;
  QIcon                          m_instructionMarkerOpenLeft;
  QIcon                          m_instructionMarkerOpenRight;
  QColor                         m_outOfRangeCol;
  QColor                         m_gridCol;
  QColor                         m_timelineBgColor;
  qint64                         m_iMaximumSizeMs = timeline::c_iDefaultLength;
  qint64                         m_iPageLengthMs = timeline::c_iDefaultStepSize;
  qint64                         m_iWindowStartMs = 0;
};

#endif // CTIMELINEWIDGETLAYERBACKGROUND_H
