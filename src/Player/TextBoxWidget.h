#ifndef TEXTBOXWIDGET_H
#define TEXTBOXWIDGET_H

#include "Widgets/IWidgetBaseInterface.h"
#include <QPointer>
#include <QWidget>
#include <memory>

class CSettings;
namespace Ui {
  class CTextBoxWidget;
}
class QPropertyAnimation;

class CTextBoxWidget : public QWidget, public IWidgetBaseInterface
{
  Q_OBJECT

public:
  explicit CTextBoxWidget(QWidget* pParent = nullptr);
  ~CTextBoxWidget() override;

  void Initialize() override;

  std::vector<QColor> BackgroundColors() { return m_vCurrentBackgroundColor; }
  std::vector<QColor> TextColors() { return m_vCurrentTextColor; }
  bool SceneSelection() { return m_bSceneSelection; }
  void SetSceneSelection(bool bSceneSelection) { m_bSceneSelection = bSceneSelection; }

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
  void SignalShowSceneSelectReturnValue(qint32 iIndex);

private slots:
  void SlotSliderRangeChanged();

private:
  void AddDropShadow(QWidget* pWidget);
  void ScrollToBottom();

  std::unique_ptr<Ui::CTextBoxWidget> m_spUi;
  std::shared_ptr<CSettings>          m_spSettings;
  QPointer<QPropertyAnimation>        m_pTextSliderValueAnimation;
  std::vector<QColor>                 m_vCurrentBackgroundColor;
  std::vector<QColor>                 m_vCurrentTextColor;
  bool                                m_bSceneSelection;
};

#endif // TEXTBOXWIDGET_H
