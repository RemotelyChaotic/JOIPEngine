#ifndef EDITORLAYOUTMODERN_H
#define EDITORLAYOUTMODERN_H

#include "EditorLayoutBase.h"
#include "Editor/EditorWidgetRegistry.h"

class CEditorLayoutClassic;
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

  QPointer<CEditorLayoutClassic> TopEditor() const;
  QPointer<QWidget> BottomEditor() const;

protected:
  void InitializeImpl(bool bWithTutorial) override;

private:
  std::unique_ptr<Ui::CEditorLayoutModern> m_spUi;
};

#endif // EDITORLAYOUTMODERN_H
