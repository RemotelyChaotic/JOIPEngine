#ifndef EDITORACTIONBAR_H
#define EDITORACTIONBAR_H

#include "EditorWidgets/EditorWidgetBase.h"
#include "enum.h"
#include <memory>

class CSettings;
namespace Ui {
  class CEditorActionBar;
}

class CEditorActionBar : public CEditorWidgetBase
{
  Q_OBJECT
  Q_PROPERTY(qint32 spacing READ Spacing WRITE SetSpacing)

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

  qint32 Spacing() const;
  void SetSpacing(qint32 iValue);
  qint32 CurrentActionBar();

  void SetActionBarPosition(EActionBarPosition position);

  void EditedProject() override {}
  void Initialize() override;
  void LoadProject(tspProject spProject) override { Q_UNUSED(spProject); }
  void UnloadProject() override {}
  void SaveProject() override {}
  void OnHidden() override {};
  void OnShown() override {};

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
  qint32                                           m_iSpacing;
};

#endif // EDITORACTIONBAR_H
