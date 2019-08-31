#include "EditorResourceDisplayWidget.h"
#include "Application.h"
#include "EditorActionBar.h"
#include "Backend/DatabaseManager.h"
#include "ui_EditorResourceDisplayWidget.h"
#include "ui_EditorActionBar.h"

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

  m_spUi->pResourceDisplay->SlotSetSliderVisible(true);

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceDisplayWidget::LoadResource(tspResource spResource)
{
  m_spUi->pResourceDisplay->LoadResource(spResource);
  UpdateActionBar();
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
  if (nullptr != ActionBar())
  {
    disconnect(ActionBar()->m_spUi->pPlayButton, &QPushButton::clicked,
            m_spUi->pResourceDisplay, &CResourceDisplayWidget::SlotPlayPause);
    disconnect(ActionBar()->m_spUi->pPauseButton, &QPushButton::clicked,
               m_spUi->pResourceDisplay, &CResourceDisplayWidget::SlotPlayPause);
    disconnect(ActionBar()->m_spUi->pStopButton, &QPushButton::clicked,
            m_spUi->pResourceDisplay, &CResourceDisplayWidget::SlotStop);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceDisplayWidget::OnActionBarChanged()
{
  // connections for actionbar
  if (nullptr != ActionBar())
  {
    UpdateActionBar();
    connect(ActionBar()->m_spUi->pPlayButton, &QPushButton::clicked,
            m_spUi->pResourceDisplay, &CResourceDisplayWidget::SlotPlayPause);
    connect(ActionBar()->m_spUi->pPauseButton, &QPushButton::clicked,
            m_spUi->pResourceDisplay, &CResourceDisplayWidget::SlotPlayPause);
    connect(ActionBar()->m_spUi->pStopButton, &QPushButton::clicked,
            m_spUi->pResourceDisplay, &CResourceDisplayWidget::SlotStop);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceDisplayWidget::UpdateActionBar()
{
  switch (m_spUi->pResourceDisplay->ResourceType())
  {
  case EResourceType::eMovie: // fallthrough
  case EResourceType::eSound:
    if (nullptr != ActionBar())
    {
      ActionBar()->ShowMediaPlayerActionBar();
    }
    break;
  default:
    if (nullptr != ActionBar())
    {
      ActionBar()->HideAllBars();
    }
    break;
  }
}
