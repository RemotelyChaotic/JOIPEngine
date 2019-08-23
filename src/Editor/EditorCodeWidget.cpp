#include "EditorCodeWidget.h"
#include "EditorActionBar.h"
#include "ui_EditorCodeWidget.h"
#include "ui_EditorActionBar.h"

CEditorCodeWidget::CEditorCodeWidget(QWidget* pParent) :
  CEditorWidgetBase(pParent),
  m_spUi(new Ui::CEditorCodeWidget),
  m_spCurrentProject(nullptr)
{
  m_spUi->setupUi(this);
}

CEditorCodeWidget::~CEditorCodeWidget()
{
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::Initialize()
{
  m_bInitialized = false;


  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::LoadProject(tspProject spProject)
{
  WIDGET_INITIALIZED_GUARD

  m_spCurrentProject = spProject;
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::UnloadProject()
{
  WIDGET_INITIALIZED_GUARD

  m_spCurrentProject = nullptr;
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::OnActionBarAboutToChange()
{
  if (nullptr != ActionBar())
  {
    //disconnect(ActionBar()->m_spUi->pPlayButton, &QPushButton::clicked,
    //        m_spUi->pResourceDisplay, &CResourceDisplayWidget::SlotPlayPause);
    //disconnect(ActionBar()->m_spUi->pStopButton, &QPushButton::clicked,
    //        m_spUi->pResourceDisplay, &CResourceDisplayWidget::SlotStop);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::OnActionBarChanged()
{
  // connections for actionbar
  if (nullptr != ActionBar())
  {
    ActionBar()->HideAllBars();
    //UpdateActionBar();
    //connect(ActionBar()->m_spUi->pPlayButton, &QPushButton::clicked,
    //        m_spUi->pResourceDisplay, &CResourceDisplayWidget::SlotPlayPause);
    //connect(ActionBar()->m_spUi->pStopButton, &QPushButton::clicked,
    //        m_spUi->pResourceDisplay, &CResourceDisplayWidget::SlotStop);
  }
}

