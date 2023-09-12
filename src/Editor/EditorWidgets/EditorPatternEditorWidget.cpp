#include "EditorPatternEditorWidget.h"

DECLARE_EDITORWIDGET(CEditorPatternEditorWidget, EEditorWidget::ePatternEditor)

CEditorPatternEditorWidget::CEditorPatternEditorWidget(QWidget* pParent) :
    CEditorWidgetBase(pParent),
    m_spUi(std::make_unique<Ui::CEditorPatternEditorWidget>())
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

}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::LoadProject(tspProject spProject)
{
  Q_UNUSED(spProject)
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
