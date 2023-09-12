#ifndef EDITORPATTERNEDITORWIDGET_H
#define EDITORPATTERNEDITORWIDGET_H

#include "EditorWidgetBase.h"
#include "ui_EditorPatternEditorWidget.h"
#include <QPointer>
#include <memory>

namespace Ui {
  class CEditorPatternEditorWidget;
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

private:
  std::shared_ptr<Ui::CEditorPatternEditorWidget> m_spUi;
};

#endif // EDITORPATTERNEDITORWIDGET_H
