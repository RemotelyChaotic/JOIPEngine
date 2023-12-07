#ifndef CTIMELINEWIDGETLAYER_H
#define CTIMELINEWIDGETLAYER_H

#include <QWidget>

class CTimelineWidgetLayer : public QWidget
{
  Q_OBJECT
public:
  explicit CTimelineWidgetLayer(QWidget *parent = nullptr);
  ~CTimelineWidgetLayer() override;

signals:

protected:
  void paintEvent(QPaintEvent* pEvt) override;

};

#endif // CTIMELINEWIDGETLAYER_H
