#ifndef CTIMELINEWIDGETCONTROLS_H
#define CTIMELINEWIDGETCONTROLS_H

#include "TimelineWidgetUtils.h"

#include <QComboBox>
#include <QFrame>
#include <QLabel>
#include <QPointer>
#include <QWidget>

class CZoomComboBox;

class CTimelineWidgetControls : public QFrame
{
  Q_OBJECT
  Q_PROPERTY(QColor cursorColor READ CursorColor WRITE SetCursorColor)

public:
  explicit CTimelineWidgetControls(QWidget* pParent);
  ~CTimelineWidgetControls() override;

  void SetCursorColor(const QColor& col);
  const QColor& CursorColor() const;
  void SetGridColor(const QColor& col);
  const QColor& GridColor() const;
  void SetOutOfRangeColor(const QColor& col);
  const QColor& OutOfRangeColor() const;

  qint64 CurrentTimeStamp() const;
  void SetCurrentCursorPos(qint32 iX);
  void SetCurrentTimeStamp(qint64 iTimeMs);
  void SetCurrentWindow(qint64 iStartMs, qint64 iPageLengthMs);
  void SetHeaderSize(QSize s);
  void SetTimeMaximum(qint64 iTimeMs);
  void ZoomIn();
  void ZoomOut();

  qint32 CursorFromCurrentTime() const;
  qint32 CurrentCursorPos() const;
  qint64 TimeFromCursor() const;
  qint32 Zoom() const;

  QSize sizeHint() const override;
  QSize minimumSizeHint() const override;

signals:
  void SignalZoomChanged(qint32 iZoom);

protected:
  void mouseMoveEvent(QMouseEvent* pEvent) override;
  void paintEvent(QPaintEvent* pEvt) override;

private slots:
  void SlotZoomChanged(qint32 iZoom);

private:
  void UpdateCurrentLabel();

  QPointer<QWidget>              m_pControls;
  QPointer<CZoomComboBox>        m_pComboZoom;
  QPointer<QLabel>               m_pCurrentLabel;
  QColor                         m_cursorCol;
  QColor                         m_gridCol;
  QColor                         m_outOfRangeCol;
  qint32                         m_iCursorPos = -1;
  qint64                         m_iCursorTime = -1;
  qint64                         m_iSelectedTime = -1;
  qint64                         m_iMaximumSizeMs = timeline::c_iDefaultLength;
  qint64                         m_iPageLengthMs = timeline::c_iDefaultStepSize;
  qint64                         m_iWindowStartMs = 0;
};

#endif // CTIMELINEWIDGETCONTROLS_H
