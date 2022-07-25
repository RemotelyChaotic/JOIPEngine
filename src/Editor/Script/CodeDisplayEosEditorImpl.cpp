#include "CodeDisplayEosEditorImpl.h"
#include "ui_EditorActionBar.h"

CCodeDisplayEosEditorImpl::CCodeDisplayEosEditorImpl(QPointer<QTreeView> pTarget) :
  ICodeDisplayWidgetImpl(),
  m_pTarget(pTarget)
{

}
CCodeDisplayEosEditorImpl::~CCodeDisplayEosEditorImpl()
{

}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayEosEditorImpl::Clear()
{
  //m_pCodeEdit->clear();
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayEosEditorImpl::ExecutionError(QString sException, qint32 iLine, QString sStack)
{
  //m_pCodeEdit->SlotExecutionError(sException, iLine, sStack);
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayEosEditorImpl::InsertGeneratedCode(const QString& sCode)
{
  //m_pCodeEdit->insertPlainText(sCode);
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayEosEditorImpl::ResetWidget()
{
  //m_pCodeEdit->ResetWidget();
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayEosEditorImpl::SetContent(const QString& sContent)
{
  //m_pCodeEdit->setPlainText(sContent);
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayEosEditorImpl::SetHighlightDefinition(const QString& sType)
{
  //m_pCodeEdit->SetHighlightDefinition(sType);
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayEosEditorImpl::ShowButtons(Ui::CEditorActionBar* pActionBar)
{
  pActionBar->AddShowBackgroundCode->setVisible(false);
  pActionBar->AddShowIconCode->setVisible(false);
  pActionBar->AddShowImageCode->setVisible(false);
  pActionBar->AddTextCode->setVisible(false);
  pActionBar->AddMetronomeCode->setVisible(false);
  pActionBar->AddNotificationCode->setVisible(false);
  pActionBar->AddTimerCode->setVisible(false);
  pActionBar->AddThreadCode->setVisible(false);
}

//----------------------------------------------------------------------------------------
//
void CCodeDisplayEosEditorImpl::Update()
{
  //m_pCodeEdit->update();
}

//----------------------------------------------------------------------------------------
//
QString CCodeDisplayEosEditorImpl::GetCurrentText() const
{
  return QString();//m_pCodeEdit->toPlainText();
}
