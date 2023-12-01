#include "CodeDisplayLayoutEditorImpl.h"
#include "ui_EditorActionBar.h"

CCodeDisplayLayoutEditorImpl::CCodeDisplayLayoutEditorImpl(QPointer<CScriptEditorWidget> pTarget) :
    CCodeDisplayDefaultEditorImpl(pTarget)
{
}

CCodeDisplayLayoutEditorImpl::~CCodeDisplayLayoutEditorImpl()
{}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayLayoutEditorImpl::HideButtons(Ui::CEditorActionBar* pActionBar)
{
  if (nullptr != pActionBar)
  {
    pActionBar->pCodeEditorContainerStack->setCurrentWidget(pActionBar->pCodeEditorContainerPageLayout);
  }
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayLayoutEditorImpl::ShowButtons(Ui::CEditorActionBar* pActionBar)
{
  if (nullptr != pActionBar)
  {
    pActionBar->pCodeEditorContainerStack->setCurrentWidget(pActionBar->pCodeEditorContainerPageLayout);
  }
}
