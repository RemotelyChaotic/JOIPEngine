#ifndef INFORMATIONWIDGET_H
#define INFORMATIONWIDGET_H

#include "Widgets/IWidgetBaseInterface.h"
#include <QWidget>
#include <memory>

class CSettings;
namespace Ui {
  class CInformationWidget;
}

class CInformationWidget : public QWidget, public IWidgetBaseInterface
{
  Q_OBJECT

public:
  explicit CInformationWidget(QWidget* pParent = nullptr);
  ~CInformationWidget() override;

  void Initialize() override;

private:
  std::unique_ptr<Ui::CInformationWidget> m_spUi;
  std::shared_ptr<CSettings>              m_spSettings;
};

#endif // INFORMATIONWIDGET_H
