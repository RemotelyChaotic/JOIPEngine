#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include <QScrollArea>
#include <memory>

namespace Ui {
  class CTimelineWidget;
}

class CTimelineWidget : public QScrollArea
{
  Q_OBJECT

public:
  explicit CTimelineWidget(QWidget* pParent = nullptr);
  ~CTimelineWidget();

  QSize minimumSizeHint() const override;
  QSize sizeHint() const override;

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  std::unique_ptr<Ui::CTimelineWidget> m_spUi;
};

#endif // TIMELINEWIDGET_H
