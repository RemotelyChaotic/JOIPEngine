#ifndef COLORPICKER_H
#define COLORPICKER_H

#include <QColor>
#include <QWidget>
#include <memory>

namespace Ui {
  class CColorPicker;
}

class CColorPicker : public QWidget
{
  Q_OBJECT

public:
  explicit CColorPicker(QWidget* pParent = nullptr);
  ~CColorPicker() override;

  void SetColor(const QColor& color);
  QColor Color() const;

  void SetAlphaVisible(bool bVisible);
  bool IsAlphaVisible();

signals:
  void SignalColorChanged(const QColor& color);

protected:
  bool eventFilter(QObject* pObject, QEvent* pEvent) override;

protected slots:
  void on_pSpinBoxR_valueChanged(int i);
  void on_pSpinBoxG_valueChanged(int i);
  void on_pSpinBoxB_valueChanged(int i);
  void on_pSpinBoxA_valueChanged(int i);

private:
  void UpdateWidgetDisplay();

  std::unique_ptr<Ui::CColorPicker> m_spUi;
  QColor                            m_color;
};

#endif // COLORPICKER_H
