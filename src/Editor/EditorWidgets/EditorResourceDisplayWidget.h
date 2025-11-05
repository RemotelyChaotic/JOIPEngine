#ifndef EDITORRESOURCEDISPLAYWIDGET_H
#define EDITORRESOURCEDISPLAYWIDGET_H

#include "EditorWidgetBase.h"
#include "Enums.h"
#include "Systems/Database/Resource.h"
#include "ui_EditorResourceDisplayWidget.h"
#include "ui_EditorActionBar.h"
#include <memory>

class CResourceDisplayTutorialStateSwitchHandler;
namespace Ui {
  class CEditorResourceDisplayWidget;
}

class CEditorResourceDisplayWidget : public CEditorWidgetBase
{
  Q_OBJECT

public:
  explicit CEditorResourceDisplayWidget(QWidget* pParent = nullptr);
  ~CEditorResourceDisplayWidget() override;

  void EditedProject() override {}
  void Initialize() override;
  void LoadProject(tspProject spProject) override { Q_UNUSED(spProject); SetLoaded(true); }
  void UnloadProject() override;
  void SaveProject() override {}
  void OnHidden() override {}
  void OnShown() override {}

  void LoadResource(tspResource spResource);
  ELoadState LoadState() const;
  void UnloadResource();
  void UpdateActionBar();

protected slots:
  void SlotLoadFinished();

protected:
  void OnActionBarAboutToChange() override;
  void OnActionBarChanged() override;

  std::shared_ptr<Ui::CEditorResourceDisplayWidget>           m_spUi;
  std::shared_ptr<CResourceDisplayTutorialStateSwitchHandler> m_spTutorialStateSwitchHandler;
};

#endif // EDITORRESOURCEDISPLAYWIDGET_H
