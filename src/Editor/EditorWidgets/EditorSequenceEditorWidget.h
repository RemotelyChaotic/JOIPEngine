#ifndef EDITORPATTERNEDITORWIDGET_H
#define EDITORPATTERNEDITORWIDGET_H

#include "EditorWidgetBase.h"
#include "ui_EditorSequenceEditorWidget.h"
#include "ui_EditorActionBar.h"
#include <QPointer>
#include <memory>

namespace Ui {
  class CEditorSequenceEditorWidget;
}

//----------------------------------------------------------------------------------------
//
class CEditorPatternEditorWidget : public CEditorWidgetBase
{
  Q_OBJECT

public:
  explicit CEditorPatternEditorWidget(QWidget* pParent = nullptr);
  ~CEditorPatternEditorWidget() override;

  void EditedProject() override {}
  void Initialize() override;
  void LoadProject(tspProject spProject) override;
  void UnloadProject() override;
  void SaveProject() override;
  void OnHidden() override;
  void OnShown() override;

protected:
  void OnActionBarAboutToChange() override;
  void OnActionBarChanged() override;

protected slots:
  void SlotAddNewSequenceButtonClicked();
  void SlotAddSequenceLayerButtonClicked();
  void SlotRemoveSequenceLayerButtonClicked();
  void SlotAddSequenceElementButtonClicked();
  void SlotRemoveSequenceElementsButtonClicked();

private:
  void OpenContextMenuAt(const QPoint& currentAddPoint, const QPoint& createPoint);

  std::shared_ptr<Ui::CEditorSequenceEditorWidget> m_spUi;
  tspProject                                       m_spCurrentProject;
};

#endif // EDITORPATTERNEDITORWIDGET_H
