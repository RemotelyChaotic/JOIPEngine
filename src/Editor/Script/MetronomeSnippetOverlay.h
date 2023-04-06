#ifndef METRONOMESNIPPETOVERLAY_H
#define METRONOMESNIPPETOVERLAY_H

#include "CodeSnippetOverlayBase.h"
#include <QScrollArea>
#include <vector>

class CDatabaseManager;
class CResourceTreeItemModel;
class CScriptEditorWidget;
namespace Ui {
  class CMetronomeSnippetOverlay;
}

//----------------------------------------------------------------------------------------
//
class CMetronomeSnippetOverlay : public CCodeSnippetOverlayBase
{
  Q_OBJECT

public:
  explicit CMetronomeSnippetOverlay(QWidget* pParent = nullptr);
  ~CMetronomeSnippetOverlay();

  void Initialize(CResourceTreeItemModel* pResourceTreeModel);

public slots:
  void Resize() override;
  void Show() override;

protected slots:
  void on_pPlayRadioButton_toggled(bool bState);
  void on_pStopRadioButton_toggled(bool bState);
  void on_pBpmCheckBox_toggled(bool bState);
  void on_pBpmSpinBox_valueChanged(int iValue);
  void on_pPatternCheckBox_toggled(bool bState);
  void on_AddPatternElemButton_clicked();
  void on_RemovePatternElemButton_clicked();
  void SlotPatternValueChanged(double dValue);
  void on_pMuteCheckBox_toggled(bool bValue);
  void on_pVolumeCheckBox_toggled(bool bChecked);
  void on_pVolumeSlider_sliderReleased();
  void on_pSetBeatSoundCheckBox_toggled(bool bState);
  void on_pResourceLineEdit_editingFinished();
  void on_pFilter_SignalFilterChanged(const QString& sText);
  void on_CloseButton_clicked();
  void on_pConfirmButton_clicked();
  void on_pCancelButton_clicked();
  void SlotCurrentChanged(const QModelIndex &current, const QModelIndex &previous);

private:
  void Initialize();

  std::unique_ptr<Ui::CMetronomeSnippetOverlay> m_spUi;
  std::weak_ptr<CDatabaseManager>               m_wpDbManager;
  std::vector<QPointer<QScrollArea>>            m_vScrollAreas;
  bool                                          m_bInitialized;
  SMetronomeSnippetCode                         m_data;
  QSize                                         m_preferredSize;
};

#endif // METRONOMESNIPPETOVERLAY_H
