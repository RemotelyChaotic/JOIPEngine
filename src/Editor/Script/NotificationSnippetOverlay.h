#ifndef NOTIFICATIONSNIPPETOVERLAY_H
#define NOTIFICATIONSNIPPETOVERLAY_H

#include "CodeSnippetOverlayBase.h"
#include <QScrollArea>
#include <QStyledItemDelegate>
#include <QPointer>
#include <vector>

class CDatabaseManager;
class CResourceTreeItemModel;
class CScriptEditorWidget;
namespace Ui {
class CNotificationSnippetOverlay;
}

//----------------------------------------------------------------------------------------
//
class CNotificationSnippetOverlay : public CCodeSnippetOverlayBase
{
  Q_OBJECT

public:
  explicit CNotificationSnippetOverlay(QWidget* pParent = nullptr);
  ~CNotificationSnippetOverlay();

  void Initialize(CResourceTreeItemModel* pResourceTreeModel);

public slots:
  void Resize() override;
  void Show() override;

protected slots:
  void on_pSetTextOrientation_toggled(bool bStatus);
  void on_pOrientationComboBox_currentIndexChanged(qint32 iIndex);
  void on_pSetTimeoutTimeCheckBox_toggled(bool bStatus);
  void on_pTimeSpinBox_valueChanged(double dValue);
  void on_pIdLineEdit_editingFinished();
  void on_pShowRadioButton_toggled(bool bStatus);
  void on_pHideRadioButton_toggled(bool bStatus);
  void on_pClearRadioButton_toggled(bool bStatus);
  void on_pTextEdit_textChanged();
  void on_pWidgetTextEdit_textChanged();

  void on_pShowIconCheckBox_toggled(bool bStatus);
  void on_pResourceLineEdit_editingFinished();
  void on_pFilter_SignalFilterChanged(const QString& sText);
  void SlotIconCurrentChanged(const QModelIndex &current, const QModelIndex &previous);

  void on_pOnClickCheckBox_toggled(bool bStatus);
  void on_pOnClickResourceLineEdit_editingFinished();
  void on_pOnClickFilter_SignalFilterChanged(const QString& sText);
  void SlotOnClickCurrentChanged(const QModelIndex &current, const QModelIndex &previous);

  void on_pOnTimeoutCheckBox_toggled(bool bStatus);
  void on_pOnTimeoutResourceLineEdit_editingFinished();
  void on_pOnTimeoutFilter_SignalFilterChanged(const QString& sText);
  void SlotOnTimeoutCurrentChanged(const QModelIndex &current, const QModelIndex &previous);

  void on_pSetTextColorCheckBox_toggled(bool bStatus);
  void on_pSetTextBackgroundColorCheckBox_toggled(bool bStatus);
  void on_pSetWidgetColorCheckBox_toggled(bool bStatus);
  void on_pSetWidgetBackgrounColorCheckBox_toggled(bool bStatus);
  void on_pTextColor_SignalColorChanged(const QColor& color);
  void on_pTextBackgroundColor_SignalColorChanged(const QColor& color);
  void on_pWidgetTextColor_SignalColorChanged(const QColor& color);
  void on_pWidgetBackgroundColor_SignalColorChanged(const QColor& color);
  void on_pConfirmButton_clicked();
  void on_pCancelButton_clicked();

private:
  void Initialize();

  std::unique_ptr<Ui::CNotificationSnippetOverlay> m_spUi;
  std::weak_ptr<CDatabaseManager>                  m_wpDbManager;
  std::vector<QPointer<QScrollArea>>               m_vScrollAreas;
  SNotificationSnippetCode                         m_data;
  QSize                                            m_preferredSize;
};

#endif // NOTIFICATIONSNIPPETOVERLAY_H
