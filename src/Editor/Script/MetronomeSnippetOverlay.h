#ifndef METRONOMESNIPPETOVERLAY_H
#define METRONOMESNIPPETOVERLAY_H

#include "Widgets/OverlayBase.h"
#include <memory>

class CDatabaseManager;
class CResourceTreeItemModel;
namespace Ui {
  class CMetronomeSnippetOverlay;
}
typedef std::shared_ptr<struct SProject> tspProject;


struct SMetronomeSnippetCode
{
  bool m_bStart = true;
  bool m_bStop = false;
  bool m_bSetBpm = false;
  qint32 m_iBpm = 60;
  bool m_bSetPattern = false;
  std::map<qint32, double> m_vdPatternElems;
  bool m_bSetBeatSound = false;
  QString m_sBeatSound = QString();
};

//----------------------------------------------------------------------------------------
//
class CMetronomeSnippetOverlay : public COverlayBase
{
  Q_OBJECT

public:
  explicit CMetronomeSnippetOverlay(QWidget* pParent = nullptr);
  ~CMetronomeSnippetOverlay();

  void Initialize(CResourceTreeItemModel* pResourceTreeModel);
  void LoadProject(tspProject spProject);
  void UnloadProject();

signals:
  void SignalMetronomeSnippetCode(const QString& code);

public slots:
  void Climb() override;
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
  tspProject                                    m_spCurrentProject;
  std::weak_ptr<CDatabaseManager>               m_wpDbManager;
  bool                                          m_bInitialized;
  SMetronomeSnippetCode                         m_data;
};

#endif // METRONOMESNIPPETOVERLAY_H
