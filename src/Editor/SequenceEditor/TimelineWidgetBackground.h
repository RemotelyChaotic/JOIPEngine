#ifndef CTIMELINEWIDGETBACKGROUND_H
#define CTIMELINEWIDGETBACKGROUND_H

#include <QPointer>
#include <QWidget>

class CTimelineWidget;

class CTimelineWidgetBackground : public QWidget
{
  Q_OBJECT

public:
  explicit CTimelineWidgetBackground(QWidget* pParent = nullptr);
  ~CTimelineWidgetBackground() override;

  void SetTimelineWidget(CTimelineWidget* pWidget);

protected:
  void paintEvent(QPaintEvent* pEvt) override;

private:
  QPointer<CTimelineWidget>            m_pWidget;
};

#endif // CTIMELINEWIDGETBACKGROUND_H
