#ifndef CTIMELINEWIDGETBACKGROUND_H
#define CTIMELINEWIDGETBACKGROUND_H

#include <QWidget>

class CTimelineWidgetBackground : public QWidget
{
  Q_OBJECT

public:
  explicit CTimelineWidgetBackground(QWidget* pParent = nullptr);
  ~CTimelineWidgetBackground() override;
};

#endif // CTIMELINEWIDGETBACKGROUND_H
