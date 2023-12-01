#ifndef CCODEDISPLAYLAYOUTEDITORIMPL_H
#define CCODEDISPLAYLAYOUTEDITORIMPL_H

#include "CodeDisplayDefaultEditorImpl.h"

class CCodeDisplayLayoutEditorImpl : public CCodeDisplayDefaultEditorImpl
{
public:
  CCodeDisplayLayoutEditorImpl(QPointer<CScriptEditorWidget> pTarget);
  ~CCodeDisplayLayoutEditorImpl() override;

  void HideButtons(Ui::CEditorActionBar* pActionBar) override;
  void ShowButtons(Ui::CEditorActionBar* pActionBar) override;
};

#endif // CCODEDISPLAYLAYOUTEDITORIMPL_H
