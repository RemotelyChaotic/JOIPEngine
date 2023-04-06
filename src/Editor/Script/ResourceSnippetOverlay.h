#ifndef RESOURCESNIPPETOVERLAY_H
#define RESOURCESNIPPETOVERLAY_H

#include "CodeSnippetOverlayBase.h"
#include <QPointer>

class CDatabaseManager;
class CResourceTreeItemModel;
class CScriptEditorWidget;
namespace Ui {
  class CResourceSnippetOverlay;
}

//----------------------------------------------------------------------------------------
//
class CResourceSnippetOverlay : public CCodeSnippetOverlayBase
{
  Q_OBJECT

public:
  explicit CResourceSnippetOverlay(QWidget* pParent = nullptr);
  ~CResourceSnippetOverlay() override;

  void Initialize(CResourceTreeItemModel* pResourceTreeModel);

public slots:
  void Resize() override;

protected slots:
  void on_pResourceLineEdit_editingFinished();
  void on_CloseButton_clicked();
  void on_pPlayRadioButton_toggled(bool bChecked);
  void on_pPauseRadioButton_toggled(bool bChecked);
  void on_pStopRadioButton_toggled(bool bChecked);
  void on_pSeekRadioButton_toggled(bool bChecked);
  void on_pSeekSpinBox_valueChanged(qint32 iValue);
  void on_pWaitForFinishedCheckBox_toggled(bool bChecked);
  void on_pVolumeCheckBox_toggled(bool bChecked);
  void on_pVolumeSlider_sliderReleased();
  void on_pLoopsCheckBox_toggled(bool bChecked);
  void on_pLoopsSpinBox_valueChanged(qint32 iValue);
  void on_pStartAtCheckBox_toggled(bool bChecked);
  void on_pStartAtSpinBox_valueChanged(qint32 iValue);
  void on_pEndAtCheckBox_toggled(bool bChecked);
  void on_pEndAtSpinBox_valueChanged(qint32 iValue);
  void on_pFilter_SignalFilterChanged(const QString& sText);
  void on_pConfirmButton_clicked();
  void on_pCancelButton_clicked();
  void SlotCurrentChanged(const QModelIndex &current, const QModelIndex &previous);

private:
  std::unique_ptr<Ui::CResourceSnippetOverlay> m_spUi;
  std::weak_ptr<CDatabaseManager>              m_wpDbManager;
  SResourceSnippetData                         m_data;
  QSize                                        m_preferredSize;
};

#endif // RESOURCESNIPPETOVERLAY_H
