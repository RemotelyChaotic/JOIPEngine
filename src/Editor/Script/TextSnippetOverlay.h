#ifndef TEXTSNIPPETOVERLAY_H
#define TEXTSNIPPETOVERLAY_H

#include "CodeSnippetOverlayBase.h"
#include <QScrollArea>
#include <QStyledItemDelegate>
#include <QPointer>
#include <memory>
#include <map>
#include <vector>

class CDatabaseManager;
class CResourceTreeItemModel;
class CScriptEditorWidget;
namespace Ui {
  class CTextSnippetOverlay;
}

//----------------------------------------------------------------------------------------
//
class CTextSnippetOverlay : public CCodeSnippetOverlayBase
{
  Q_OBJECT

public:
  explicit CTextSnippetOverlay(QWidget* pParent = nullptr);
  ~CTextSnippetOverlay() override;

  void Initialize(CResourceTreeItemModel* pResourceTreeModel);

public slots:
  void Resize() override;
  void Show() override;
  
protected slots:
  void on_pShowTextCheckBox_toggled(bool bStatus);
  void on_pShowUserInputCheckBox_toggled(bool bStatus);
  void on_pSetTextOrientation_toggled(bool bStatus);
  void on_pOrientationComboBox_currentIndexChanged(qint32 iIndex);
  void on_pSetSleepTimeCheckBox_toggled(bool bStatus);
  void on_pAutoTimeCheckBox_toggled(bool bStatus);
  void on_pSleepSpinBox_valueChanged(double dValue);
  void on_pSkippableCheckBox_toggled(bool bStatus);
  void on_pTextEdit_textChanged();
  void on_pShowIconCheckBox_toggled(bool bStatus);
  void on_pResourceLineEdit_editingFinished();
  void on_pFilter_SignalFilterChanged(const QString& sText);
  void SlotCurrentChanged(const QModelIndex &current, const QModelIndex &previous);
  void on_pShowButtonsCheckBox_toggled(bool bStatus);
  void on_AddButtonButton_clicked();
  void on_RemoveButtonButton_clicked();
  void SlotItemListCommitData(QWidget* pLineEdit);
  void on_pSetTextColorsCheckBox_toggled(bool bStatus);
  void on_AddTextColorButton_clicked();
  void on_RemoveTextColorButton_clicked();
  void SlotTextColorChanged(const QColor& color);
  void on_pSetBGCheckBox_toggled(bool bStatus);
  void on_AddBGColorButton_clicked();
  void on_RemoveBGColorButton_clicked();
  void SlotBGColorChanged(const QColor& color);
  void on_pConfirmButton_clicked();
  void on_pCancelButton_clicked();

private:
  void Initialize();

  std::unique_ptr<Ui::CTextSnippetOverlay> m_spUi;
  std::weak_ptr<CDatabaseManager>          m_wpDbManager;
  std::vector<QPointer<QScrollArea>>       m_vScrollAreas;
  STextSnippetCode                         m_data;
  QSize                                    m_preferredSize;
};

#endif // TEXTSNIPPETOVERLAY_H
