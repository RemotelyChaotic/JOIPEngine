#ifndef EDITORLAYOUTCOMPACT_H
#define EDITORLAYOUTCOMPACT_H

#include "EditorLayoutBase.h"
#include "Editor/EditorWidgetRegistry.h"

namespace Ui {
class CEditorLayoutCompact;
}

class CEditorLayoutCompact : public CEditorLayoutBase
{
  Q_OBJECT

public:
  explicit CEditorLayoutCompact(QWidget *parent = nullptr);
  ~CEditorLayoutCompact();

  void ProjectLoaded(tspProject spCurrentProject, bool bModified) override;
  void ProjectUnloaded() override;

  void ChangeView(qint32 iView);
  void SetButtonVisible(qint32 iView, bool bVisible);

signals:
  void SignalViewChanged(qint32 iView);

protected slots:
  void SlotViewSwitchButtonClicked();

protected:
  void InitializeImpl(bool bWithTutorial) override;

private:
  std::unique_ptr<Ui::CEditorLayoutCompact> m_spUi;
  tspProject                                m_spCurrentProject;
  std::vector<QPointer<QAction>>            m_vpKeyBindingActions;
  EEditorWidget                             m_iCurrentView;
};

DECLARE_EDITORLAYOUT(CEditorLayoutCompact, CSettings::eCompact)

#endif // EDITORLAYOUTCOMPACT_H
