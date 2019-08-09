#include "EditorResourceDisplayWidget.h"
#include "Application.h"
#include "EditorActionBar.h"
#include "Backend/DatabaseManager.h"
#include "ui_EditorResourceDisplayWidget.h"

CEditorResourceDisplayWidget::CEditorResourceDisplayWidget(QWidget* pParent) :
  CEditorWidgetBase(pParent),
  m_spUi(std::make_unique<Ui::CEditorResourceDisplayWidget>())
{
  m_spUi->setupUi(this);
}

CEditorResourceDisplayWidget::~CEditorResourceDisplayWidget()
{
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceDisplayWidget::Initialize()
{
  m_bInitialized = false;

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceDisplayWidget::LoadResource(tspResource spResource)
{
  m_spUi->pResourceDisplay->LoadResource(spResource);
}

//----------------------------------------------------------------------------------------
//
ELoadState CEditorResourceDisplayWidget::LoadState() const
{
  return m_spUi->pResourceDisplay->LoadState();
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceDisplayWidget::UnloadResource()
{
  m_spUi->pResourceDisplay->UnloadResource();
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceDisplayWidget::OnActionBarAboutToChange()
{
  // Nothing to do
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceDisplayWidget::OnActionBarChanged()
{
  // connections for actionbar
  if (nullptr != ActionBar())
  {
    ActionBar()->HideAllBars();
  }
}
