#ifndef EDITORACTIONBAR_H
#define EDITORACTIONBAR_H

#include "EditorWidgetBase.h"
#include "enum.h"
#include <memory>

namespace Ui {
  class CEditorActionBar;
}

class CEditorActionBar : public CEditorWidgetBase
{
  Q_OBJECT

public:
  explicit CEditorActionBar(QWidget* pParent = nullptr);
  ~CEditorActionBar() override;

  void EditedProject() override {}
  void Initialize() override;
  void LoadProject(tspProject spProject) override { Q_UNUSED(spProject); }
  void UnloadProject() override {}
  void SaveProject() override {}

  void HideAllBars();
  void ShowCodeActionBar();
  void ShowNodeEditorActionBar();
  void ShowMediaPlayerActionBar();
  void ShowProjectActionBar();
  void ShowProjectSettingsActionBar();
  void ShowResourceActionBar();

  std::unique_ptr<Ui::CEditorActionBar>            m_spUi;
};

#endif // EDITORACTIONBAR_H
