#ifndef TEXTBOXWIDGET_H
#define TEXTBOXWIDGET_H

#include "Widgets/IWidgetBaseInterface.h"
#include <QWidget>
#include <memory>

class CSettings;
namespace Ui {
  class CTextBoxWidget;
}

class CTextBoxWidget : public QWidget, public IWidgetBaseInterface
{
  Q_OBJECT

public:
  explicit CTextBoxWidget(QWidget* pParent = nullptr);
  ~CTextBoxWidget() override;

  void Initialize() override;

private:
  std::unique_ptr<Ui::CTextBoxWidget> m_spUi;
  std::shared_ptr<CSettings>          m_spSettings;
};

#endif // TEXTBOXWIDGET_H
