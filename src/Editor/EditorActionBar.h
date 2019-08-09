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

  void Initialize() override;

  void HideAllBars();
  void ShowProjectActionBar();
  void ShowResourceActionBar();

  std::unique_ptr<Ui::CEditorActionBar>            m_spUi;
};

#endif // EDITORACTIONBAR_H
