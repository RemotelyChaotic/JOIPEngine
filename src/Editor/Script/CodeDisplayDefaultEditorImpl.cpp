#include "CodeDisplayDefaultEditorImpl.h"
#include "ScriptEditorWidget.h"
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
  m_pCodeEdit->ResetWidget();
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayDefaultEditorImpl::SetContent(const QString& sContent)
{
  m_pCodeEdit->setPlainText(sContent);
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayDefaultEditorImpl::SetHighlightDefinition(const QString& sType)
{
  m_pCodeEdit->SetHighlightDefinition(sType);
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayDefaultEditorImpl::ShowButtons(Ui::CEditorActionBar* pActionBar)
{
  pActionBar->AddShowBackgroundCode->setVisible(true);
  pActionBar->AddShowIconCode->setVisible(true);
  pActionBar->AddShowImageCode->setVisible(true);
  pActionBar->AddTextCode->setVisible(true);
  pActionBar->AddMetronomeCode->setVisible(true);
  pActionBar->AddNotificationCode->setVisible(true);
  pActionBar->AddTimerCode->setVisible(true);
  pActionBar->AddThreadCode->setVisible(true);
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
