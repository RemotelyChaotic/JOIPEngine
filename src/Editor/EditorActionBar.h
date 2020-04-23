#ifndef EDITORACTIONBAR_H
#define EDITORACTIONBAR_H

#include "EditorWidgetBase.h"
#include "enum.h"
#include <memory>

class CSettings;
namespace Ui {
  class CEditorActionBar;
}

class CEditorActionBar : public CEditorWidgetBase
{
  Q_OBJECT

public:
  explicit CEditorActionBar(QWidget* pParent = nullptr);
  ~CEditorActionBar() override;

  enum EActionBarPosition : qint32
  {
    eNone = 0,
    eTop,
    eLeft,
    eRight
  };

  void SetActionBarPosition(EActionBarPosition position);

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

protected slots:
  void SlotKeyBindingsChanged();

private:
  std::shared_ptr<CSettings>                       m_spSettings;
  EActionBarPosition                               m_position;
  qint32                                           m_iCurrentDisplayType; // EEditorWidget
};

#endif // EDITORACTIONBAR_H
