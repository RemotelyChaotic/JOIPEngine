#ifndef EDITORCODEWIDGET_H
#define EDITORCODEWIDGET_H

#include "EditorWidgetBase.h"
#include <memory>

namespace Ui {
  class CEditorCodeWidget;
}

class CEditorCodeWidget : public CEditorWidgetBase
{
  Q_OBJECT

public:
  explicit CEditorCodeWidget(QWidget* pParent = nullptr);
  ~CEditorCodeWidget() override;

  void Initialize() override;
  void LoadProject(tspProject spProject) override;
  void UnloadProject() override;

protected:
  void OnActionBarAboutToChange() override;
  void OnActionBarChanged() override;

private:
  std::unique_ptr<Ui::CEditorCodeWidget>     m_spUi;
  tspProject                                 m_spCurrentProject;
};

#endif // EDITORCODEWIDGET_H
