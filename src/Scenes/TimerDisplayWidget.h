#ifndef TIMERWIDGET_H
#define TIMERWIDGET_H

#include "Widgets/IWidgetBaseInterface.h"
#include <QWidget>
#include <memory>

class CSettings;
namespace Ui {
  class CTimerDisplayWidget;
}
class QTimer;

class CTimerDisplayWidget : public QWidget, public IWidgetBaseInterface
{
  Q_OBJECT

public:
  explicit CTimerDisplayWidget(QWidget* pParent = nullptr);
  ~CTimerDisplayWidget() override;

  void Initialize() override;

signals:
  void SignalTimerFinished();

public slots:
  void SlotHideTimer();
  void SlotSetTime(qint32 iTimeS);
  void SlotSetTimeVisible(bool bVisible);
  void SlotShowTimer();
  void SlotStartTimer();
  void SlotStopTimer();
  void SlotWaitForTimer();

private:
  void AddDropShadow(QWidget* pWidget);

  std::unique_ptr<Ui::CTimerDisplayWidget>   m_spUi;
  std::unique_ptr<QTimer>                    m_spTimer;
  std::shared_ptr<CSettings>                 m_spSettings;
};

#endif // TIMERWIDGET_H
