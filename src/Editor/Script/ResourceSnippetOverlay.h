#ifndef RESOURCESNIPPETOVERLAY_H
#define RESOURCESNIPPETOVERLAY_H

#include "Widgets/OverlayBase.h"
#include <QPointer>
#include <memory>

class CDatabaseManager;
class CResourceTreeItemModel;
class CScriptEditorWidget;
namespace Ui {
  class CResourceSnippetOverlay;
}
typedef std::shared_ptr<struct SProject> tspProject;


enum class EDisplayMode : qint32
{
  ePlayShow,
  ePause,
  eStop,
  eSeek
};

struct SResourceSnippetData
{
  QString m_sResource;
  EDisplayMode m_displayMode = EDisplayMode::ePlayShow;
  bool m_bWaitForFinished = false;
  qint32 m_iSeekTime = 0;
  bool m_bLoops = false;
  qint64 m_iLoops = 1;
  bool m_bStartAt = false;
  qint64 m_iStartAt = 0;
  bool m_bEndAt = false;
  qint64 m_iEndAt = -1;
  bool m_bSetVolume = false;
  double m_dVolume = 1.0;
};

//----------------------------------------------------------------------------------------
//
class CResourceSnippetOverlay : public COverlayBase
{
  Q_OBJECT

public:
  explicit CResourceSnippetOverlay(QWidget* pParent = nullptr);
  ~CResourceSnippetOverlay() override;

  void Initialize(CResourceTreeItemModel* pResourceTreeModel);
  void LoadProject(tspProject spProject);
  void UnloadProject();

signals:
  void SignalResourceCode(const QString& code);

public slots:
  void Climb() override;
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
  tspProject                                   m_spCurrentProject;
  std::weak_ptr<CDatabaseManager>              m_wpDbManager;
  bool                                         m_bInitialized;
  SResourceSnippetData                         m_data;
  QSize                                        m_preferredSize;
};

#endif // RESOURCESNIPPETOVERLAY_H
