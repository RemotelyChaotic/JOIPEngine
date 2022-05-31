#ifndef NOTIFICATIONSNIPPETOVERLAY_H
#define NOTIFICATIONSNIPPETOVERLAY_H

#include "Widgets/OverlayBase.h"
#include <QScrollArea>
#include <QStyledItemDelegate>
#include <QPointer>
#include <memory>
#include <vector>

class CDatabaseManager;
class CResourceTreeItemModel;
class CScriptEditorWidget;
namespace Ui {
class CNotificationSnippetOverlay;
}
typedef std::shared_ptr<struct SProject> tspProject;


enum EDisplayStatus
{
  eShow,
  eHide,
  eClear
};

struct SNotificationSnippetCode
{
  QString m_sId = QString();
  EDisplayStatus m_displayStatus = EDisplayStatus::eShow;
  QString m_sText = QString();
  QString m_sWidgetText = QString();
  bool m_bSetAlignment = false;
  Qt::AlignmentFlag m_textAlignment = Qt::AlignHCenter;
  bool m_bSetTimeoutTime = false;
  double m_dTimeoutTimeS = -1;
  bool m_bShowIcon = false;
  QString m_sIcon = QString();
  bool m_bOnButton = false;
  QString m_sOnButton = QString();
  bool m_bOnTimeout = false;
  QString m_sOnTimeout = QString();
  bool m_bSetTextColor = false;
  QColor m_textColor;
  bool m_bSetTextBackgroundColor = false;
  QColor m_textBackgroundColor;
  bool m_bSetWidgetTextColors = false;
  QColor m_widgetTextColor;
  bool m_bSetWidgetTextBackgroundColor = false;
  QColor m_widgettextBackgroundColor;
};

//----------------------------------------------------------------------------------------
//
class CNotificationSnippetOverlay : public COverlayBase
{
  Q_OBJECT

public:
  explicit CNotificationSnippetOverlay(CScriptEditorWidget* pParent = nullptr);
  ~CNotificationSnippetOverlay();

  void Initialize(CResourceTreeItemModel* pResourceTreeModel);
  void LoadProject(tspProject spProject);
  void UnloadProject();

signals:
  void SignalNotificationSnippetCode(const QString& code);

public slots:
  void Climb() override;
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
  tspProject                                       m_spCurrentProject;
  std::weak_ptr<CDatabaseManager>                  m_wpDbManager;
  QPointer<CScriptEditorWidget>                    m_pEditor;
  std::vector<QPointer<QScrollArea>>               m_vScrollAreas;
  bool                                             m_bInitialized;
  SNotificationSnippetCode                         m_data;
  QSize                                            m_preferredSize;
};

#endif // NOTIFICATIONSNIPPETOVERLAY_H
