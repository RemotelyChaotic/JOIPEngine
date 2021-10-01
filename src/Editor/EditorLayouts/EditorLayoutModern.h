#ifndef EDITORLAYOUTMODERN_H
#define EDITORLAYOUTMODERN_H

#include "EditorLayoutBase.h"
#include "Editor/EditorWidgetRegistry.h"

namespace Ui {
class CEditorLayoutModern;
}

class CEditorLayoutModern : public CEditorLayoutBase
{
  Q_OBJECT

public:
  explicit CEditorLayoutModern(QWidget *parent = nullptr);
  ~CEditorLayoutModern();

  void ProjectLoaded(tspProject spCurrentProject, bool bModified) override;
  void ProjectUnloaded() override;

protected:
  void InitializeImpl() override;

private:
  std::unique_ptr<Ui::CEditorLayoutModern> m_spUi;
};

DECLARE_EDITORLAYOUT(CEditorLayoutModern, CSettings::eModern)

#endif // EDITORLAYOUTMODERN_H
