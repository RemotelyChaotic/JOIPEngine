#include "ThreadSnippetOverlay.h"
#include "ScriptEditorWidget.h"
#include "ui_ThreadSnippetOverlay.h"

CThreadSnippetOverlay::CThreadSnippetOverlay(CScriptEditorWidget* pParent) :
  COverlayBase(0, pParent),
  m_spUi(std::make_unique<Ui::CThreadSnippetOverlay>()),
  m_pEditor(pParent)
{
  m_spUi->setupUi(this);
}

CThreadSnippetOverlay::~CThreadSnippetOverlay()
{
}

//----------------------------------------------------------------------------------------
//
void CThreadSnippetOverlay::Climb()
{
  if (m_pEditor->size().height() < sizeHint().height())
  {
    ClimbToFirstInstanceOf("QStackedWidget", false);
  }
  else
  {
    ClimbToFirstInstanceOf("CScriptEditorWidget", false);
  }
}

//----------------------------------------------------------------------------------------
//
void CThreadSnippetOverlay::Resize()
{
  QPoint newPos =
      QPoint(m_pTargetWidget->geometry().width() / 2, m_pTargetWidget->geometry().height() / 2) -
      QPoint(width() / 2, height() / 2);

  move(newPos.x(), newPos.y());
  resize(width(), height());
}

//----------------------------------------------------------------------------------------
//
void CThreadSnippetOverlay::on_pSleepSpinBox_valueChanged(double dValue)
{
  m_bSleepTimeS = dValue;
}

//----------------------------------------------------------------------------------------
//
void CThreadSnippetOverlay::on_pSkippableCheckBox_toggled(bool bStatus)
{
  m_bSkippable = bStatus;
}

//----------------------------------------------------------------------------------------
//
void CThreadSnippetOverlay::on_pConfirmButton_clicked()
{
  QString sCode = QString("thread.sleep(%1,%2);\n")
      .arg(m_bSleepTimeS)
      .arg(m_bSkippable ? "true" : "false");
  emit SignalThreadCode(sCode);
  Hide();
}

//----------------------------------------------------------------------------------------
//
void CThreadSnippetOverlay::on_pCancelButton_clicked()
{
  Hide();
}
