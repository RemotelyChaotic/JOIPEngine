#ifndef CTIMELINEWIDGETOVERLAY_H
#define CTIMELINEWIDGETOVERLAY_H

#include <QPointer>
#include <QWidget>

class CTimelineWidget;

class CTimelineWidgetOverlay : public QWidget
{
  Q_OBJECT
public:
  explicit CTimelineWidgetOverlay(CTimelineWidget* pParent = nullptr);
  ~CTimelineWidgetOverlay() override;

  void SetDropIndicationColor(const QColor& col);
  const QColor& DropIndicationColor() const;

  qint32 CurrentDropIndex() const;
  void SetShowDropIndicator(bool bShow);
  void SetCurrentTimePosIndicator(qint32 iPos);
  void UpdateDropLine();

protected:
  void paintEvent(QPaintEvent* pEvent) override;

private:
  std::pair<qint32, QRect> ItemAt(QPoint p);
  QLine LineFrom(QRect rect, QPoint p, bool* bBefore);

  QPointer<CTimelineWidget>            m_pParent;
  QColor                               m_pDropIndicationColor;
  QLine                                m_lineDrop;
  qint32                               m_iCurrentDropIndex = -1;
  qint32                               m_iCurrentTimePosIndicator = -1;
  bool                                 m_bShowDropIndicator = false;
};

#endif // CTIMELINEWIDGETOVERLAY_H
