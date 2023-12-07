#include "EditorSequenceEditorWidget.h"

DECLARE_EDITORWIDGET(CEditorPatternEditorWidget, EEditorWidget::ePatternEditor)

CEditorPatternEditorWidget::CEditorPatternEditorWidget(QWidget* pParent) :
    CEditorWidgetBase(pParent),
    m_spUi(std::make_unique<Ui::CEditorSequenceEditorWidget>())
{
  m_spUi->setupUi(this);
}

CEditorPatternEditorWidget::~CEditorPatternEditorWidget()
{
}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::Initialize()
{
  m_spUi->pSequenceElemsView->Initialize();
}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::LoadProject(tspProject spProject)
{
  Q_UNUSED(spProject)

  m_spUi->pTopSplitter->setSizes({ width()/4, width() *3/4 });
  m_spUi->pBottomSplitter->setSizes({ height() *3/4, height()/4 });
}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::UnloadProject()
{

}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::SaveProject()
{

}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::OnHidden()
{

}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::OnShown()
{

}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::OnActionBarAboutToChange()
{

}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::OnActionBarChanged()
{

}
