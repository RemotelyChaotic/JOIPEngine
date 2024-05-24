#include "CodeDisplayDefaultEditorImpl.h"
#include "ScriptEditorCompleter.h"
#include "ScriptEditorCompleterModel.h"
#include "ScriptEditorWidget.h"

#include "Editor/EditorModel.h"

#include "ui_EditorActionBar.h"

CCodeDisplayDefaultEditorImpl::CCodeDisplayDefaultEditorImpl(QPointer<CScriptEditorWidget> pTarget) :
  ICodeDisplayWidgetImpl(),
  m_pCodeEdit(pTarget)
{

}
CCodeDisplayDefaultEditorImpl::~CCodeDisplayDefaultEditorImpl()
{

}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayDefaultEditorImpl::Initialize(CEditorModel* pModel)
{
  if (nullptr != pModel)
  {
    m_pCodeEdit->Completer()->SetModel(pModel->EditorCompleterModel());
  }
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayDefaultEditorImpl::Clear()
{
  m_pCodeEdit->clear();
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayDefaultEditorImpl::ExecutionError(QString sException, qint32 iLine, QString sStack)
{
  m_pCodeEdit->SlotExecutionError(sException, iLine, sStack);
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayDefaultEditorImpl::InsertGeneratedCode(const QString& sCode)
{
  m_pCodeEdit->insertPlainText(sCode);
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayDefaultEditorImpl::ResetWidget()
{
  m_pCodeEdit->ResetAddons();
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayDefaultEditorImpl::SetContent(const QString& sContent)
{
  m_pCodeEdit->SetText(sContent);
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayDefaultEditorImpl::SetHighlightDefinition(const QString& sType)
{
  m_pCodeEdit->SetHighlightDefinition(sType);
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayDefaultEditorImpl::HideButtons(Ui::CEditorActionBar*)
{
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayDefaultEditorImpl::ShowButtons(Ui::CEditorActionBar* pActionBar)
{
  pActionBar->pCodeEditorContainerStack->setCurrentWidget(pActionBar->pCodeEditorContainerPageDefault);
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayDefaultEditorImpl::Update()
{
  m_pCodeEdit->update();
}

//----------------------------------------------------------------------------------------
//
QString CCodeDisplayDefaultEditorImpl::GetCurrentText() const
{
  return m_pCodeEdit->toPlainText();
}
