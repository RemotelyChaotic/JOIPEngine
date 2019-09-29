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

  std::vector<QColor> BackgroundColors() { return m_vCurrentBackgroundColor; }
  std::vector<QColor> TextColors() { return m_vCurrentTextColor; }

public slots:
  void SlotClearText();
  void SlotOnButtonPromptClicked();
  void SlotOnInputEditingFinished();
  void SlotShowButtonPrompts(QStringList vsLabels);
  void SlotShowInput();
  void SlotShowText(QString sText);
  void SlotTextBackgroundColorsChanged(std::vector<QColor> vColors);
  void SlotTextColorsChanged(std::vector<QColor> vColors);

signals:
  void SignalShowButtonReturnValue(qint32 iIndex);
  void SignalShowInputReturnValue(QString sValue);

private slots:
  void SlotSliderRangeChanged();

private:
  void AddDropShadow(QWidget* pWidget);

  std::unique_ptr<Ui::CTextBoxWidget> m_spUi;
  std::shared_ptr<CSettings>          m_spSettings;
  std::vector<QColor>                 m_vCurrentBackgroundColor;
  std::vector<QColor>                 m_vCurrentTextColor;
};

#endif // TEXTBOXWIDGET_H
