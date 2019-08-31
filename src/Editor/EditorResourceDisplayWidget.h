#ifndef EDITORRESOURCEDISPLAYWIDGET_H
#define EDITORRESOURCEDISPLAYWIDGET_H

#include "EditorWidgetBase.h"
#include "Enums.h"
#include "Backend/Resource.h"
#include <memory>

namespace Ui {
  class CEditorResourceDisplayWidget;
}

class CEditorResourceDisplayWidget : public CEditorWidgetBase
{
  Q_OBJECT

public:
  explicit CEditorResourceDisplayWidget(QWidget* pParent = nullptr);
  ~CEditorResourceDisplayWidget() override;

  void Initialize() override;
  void LoadProject(tspProject spProject) override { Q_UNUSED(spProject); }
  void UnloadProject() override {}
  void SaveProject() override {}

  void LoadResource(tspResource spResource);
  ELoadState LoadState() const;
  void UnloadResource();

protected:
  void OnActionBarAboutToChange() override;
  void OnActionBarChanged() override;

private:
  void UpdateActionBar();

  std::unique_ptr<Ui::CEditorResourceDisplayWidget> m_spUi;
};

#endif // EDITORRESOURCEDISPLAYWIDGET_H
