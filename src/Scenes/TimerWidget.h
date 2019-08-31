#ifndef TIMERWIDGET_H
#define TIMERWIDGET_H

#include "Widgets/IWidgetBaseInterface.h"
#include <QWidget>
#include <memory>

class CSettings;
namespace Ui {
  class CTimerWidget;
}

class CTimerWidget : public QWidget, public IWidgetBaseInterface
{
  Q_OBJECT

public:
  explicit CTimerWidget(QWidget* pParent = nullptr);
  ~CTimerWidget() override;

  void Initialize() override;

private:
  std::unique_ptr<Ui::CTimerWidget>   m_spUi;
  std::shared_ptr<CSettings>          m_spSettings;
};

#endif // TIMERWIDGET_H
