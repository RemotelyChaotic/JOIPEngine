#ifndef CTIMELINEWIDGETLAYERBACKGROUND_H
#define CTIMELINEWIDGETLAYERBACKGROUND_H

#include "TimelineWidgetUtils.h"

#include "Systems/Sequence/Sequence.h"

#include <QFrame>
#include <QIcon>

#include <tuple>

class CTimelineWidgetLayerBackground : public QFrame
{
  Q_OBJECT
  Q_PROPERTY(qint32 iconWidth READ IconWidth WRITE SetIconWidth)
  Q_PROPERTY(qint32 iconHeight READ IconHeight WRITE SetIconHeight)
  Q_PROPERTY(QColor instructionConnectorColor READ InstructionConnectorColor WRITE SetInstructionConnectorColor)
  Q_PROPERTY(QIcon instructionInvalidMarker READ InstructionInvalidMarker WRITE SetInstructionInvalidMarker)
  Q_PROPERTY(QIcon instructionMarker READ InstructionMarker WRITE SetInstructionMarker)
  Q_PROPERTY(QIcon instructionMarkerOpenLeft READ InstructionMarkerOpenLeft WRITE SetInstructionMarkerOpenLeft)
  Q_PROPERTY(QIcon instructionMarkerOpenRight READ InstructionMarkerOpenRight WRITE SetInstructionMarkerOpenRight)

public:
  explicit CTimelineWidgetLayerBackground(QWidget* pParent = nullptr);
  ~CTimelineWidgetLayerBackground() override;

  void SetGridColor(const QColor& col);
  const QColor& GridColor() const;
  void SetIconWidth(qint32 iWidth);
  qint32 IconWidth() const;
  void SetIconHeight(qint32 iHeight);
  qint32 IconHeight() const;
  void SetInstructionConnectorColor(const QColor& col);
  const QColor& InstructionConnectorColor() const;
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
  void SetSelectionColor(const QColor& col);
  const QColor& SelectionColor() const;
  void SetTimelineBackgroundColor(const QColor& col);
  const QColor& TimelineBackgroundColor() const;

  void ClearSelection();
  qint64 CurrentlySelectedInstructionTime() const;
  std::shared_ptr<SSequenceInstruction> CurrentlySelectedInstruction() const;
  std::shared_ptr<SSequenceInstruction> InstructionFromTime(qint64 iTime) const;

  void SetLayer(const tspSequenceLayer& spLayer);
  void SetCurrentWindow(qint64 iStartMs, qint64 iPageLengthMs);
  void SetSelectedInstruction(qint64 iIdx);
  void SetTimeGrid(qint64 iGrid);
  void SetTimeMaximum(qint64 iTimeMs);

signals:
  void SignalEditInstruction(qint64 iInstr);
  void SignalOpenInsertContextMenuAt(QPoint p, qint64 iCursorTime);

protected:
  bool eventFilter(QObject* pObj, QEvent* pEvt) override;
  void paintEvent(QPaintEvent* pEvt) override;
  void mouseMoveEvent(QMouseEvent* pMouse) override;
  void mouseReleaseEvent(QMouseEvent* pEvt) override;

private:
  qint64 ConstrainToGrid(qint64 iValue);
  std::tuple<qint64, qint64, qint64> GetTimeRangeAroundCursor(QPoint p) const;
  qint64 MostLikelyInstruction(const std::tuple<qint64, qint64, qint64>& timeRange) const;

  tspSequenceLayer               m_spLayer;
  QIcon                          m_instructionInvalidMarker;
  QIcon                          m_instructionMarker;
  QIcon                          m_instructionMarkerOpenLeft;
  QIcon                          m_instructionMarkerOpenRight;
  QColor                         m_instructionConnectorColor;
  QColor                         m_outOfRangeCol;
  QColor                         m_gridCol;
  QColor                         m_timelineBgColor;
  QColor                         m_selectionColor;
  qint64                         m_iTimeGrid = 100;
  qint64                         m_iSelectedInstr = -1;
  qint64                         m_iHighlightedInstr = -1;
  qint64                         m_iMaximumSizeMs = timeline::c_iDefaultLength;
  qint64                         m_iPageLengthMs = timeline::c_iDefaultStepSize;
  qint64                         m_iWindowStartMs = 0;
  qint32                         m_iIconWidth = 10;
  qint32                         m_iIconHeight = 10;

};

#endif // CTIMELINEWIDGETLAYERBACKGROUND_H
