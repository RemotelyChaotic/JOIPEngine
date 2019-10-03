#ifndef TEXTSNIPPETOVERLAY_H
#define TEXTSNIPPETOVERLAY_H

#include "Widgets/OverlayBase.h"
#include <QStyledItemDelegate>
#include <QPointer>
#include <memory>

namespace Ui {
  class CTextSnippetOverlay;
}

struct STextSnippetCode
{
  bool m_bShowText = false;
  bool m_bShowUserInput = false;
  QString m_sText = QString();
  bool m_bShowButtons = false;
  std::vector<QString> m_vsButtons;
  bool m_bSetTextColors = false;
  std::vector<QColor> m_vTextColors;
  bool m_bSetBGColors = false;
  std::vector<QColor> m_vBGColors;
};

//----------------------------------------------------------------------------------------
//
class CTextSnippetOverlay : public COverlayBase
{
  Q_OBJECT

public:
  explicit CTextSnippetOverlay(QWidget* pParent = nullptr);
  ~CTextSnippetOverlay() override;

signals:
  void SignalTextSnippetCode(const QString& code);

public slots:
  void Resize() override;
  void Show() override;
  
protected slots:
  void on_pShowTextCheckBox_toggled(bool bStatus);
  void on_pShowUserInputCheckBox_toggled(bool bStatus);
  void on_pTextEdit_textChanged();
  void on_pShowButtonsCheckBox_toggled(bool bStatus);
  void on_pAddButtonButton_clicked();
  void on_pRemoveButtonButton_clicked();
  void SlotItemListCommitData(QWidget* pLineEdit);
  void on_pSetTextColorsCheckBox_toggled(bool bStatus);
  void on_pAddTextColorButton_clicked();
  void on_pRemoveTextColorButton_clicked();
  void SlotTextColorChanged(const QColor& color);
  void on_pSetBGCheckBox_toggled(bool bStatus);
  void on_pAddBGColorButton_clicked();
  void on_pRemoveBGColorButton_clicked();
  void SlotBGColorChanged(const QColor& color);
  void on_pConfirmButton_clicked();
  void on_pCancelButton_clicked();

private:
  void Initialize();

  std::unique_ptr<Ui::CTextSnippetOverlay> m_spUi;
  bool                                     m_bInitialized;
  STextSnippetCode                         m_data;
};

#endif // TEXTSNIPPETOVERLAY_H
