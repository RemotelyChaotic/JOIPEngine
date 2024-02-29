#ifndef EDITORPATTERNEDITORWIDGET_H
#define EDITORPATTERNEDITORWIDGET_H

#include "EditorDebuggableWidget.h"
#include "ui_EditorSequenceEditorWidget.h"
#include "ui_EditorActionBar.h"

#include <QPointer>
#include <QStandardItemModel>
#include <memory>

class CFilteredEditorEditableFileModel;
class CSequencePropertiesOverlay;
namespace Ui {
  class CEditorSequenceEditorWidget;
}
typedef std::shared_ptr<struct SSequenceFile> tspSequence;

//----------------------------------------------------------------------------------------
//
class CEditorPatternEditorWidget : public CEditorDebuggableWidget
{
  Q_OBJECT

public:
  explicit CEditorPatternEditorWidget(QWidget* pParent = nullptr);
  ~CEditorPatternEditorWidget() override;

  void EditedProject() override {}
  void Initialize() override;
  void LoadProject(tspProject spProject) override;
  void UnloadProjectImpl() override;
  void SaveProject() override;
  void OnHidden() override;
  void OnShown() override;

protected:
  void OnActionBarAboutToChange() override;
  void OnActionBarChanged() override;

protected slots:
  void on_pResourceComboBox_currentIndexChanged(qint32 iIndex);
  void SlotAddNewSequenceButtonClicked();
  void SlotEditSequenceButtonClicked();
  void SlotAddSequenceLayerButtonClicked();
  void SlotContentsChange();
  void SlotFileChangedExternally(const QString& sName);
  void SlotRemoveSequenceLayerButtonClicked();
  void SlotAddSequenceElementButtonClicked();
  void SlotRemoveSequenceElementsButtonClicked();

private:
  QString CachedResourceName(qint32 iIndex);
  tSceneToDebug GetSequenceScene();
  void ReloadEditor(qint32 iIndex);

  std::unique_ptr<Ui::CEditorSequenceEditorWidget> m_spUi;
  std::unique_ptr<CSequencePropertiesOverlay>      m_spOverlayProps;
  tspSequence                                      m_spCurrentSequence;
  QPointer<CFilteredEditorEditableFileModel>       m_pFilteredScriptModel;
  QPointer<QStandardItemModel>                     m_pDummyModel;
  QString                                          m_sLastCachedSequence;
  bool                                             m_bChangingIndex;
};

#endif // EDITORPATTERNEDITORWIDGET_H
