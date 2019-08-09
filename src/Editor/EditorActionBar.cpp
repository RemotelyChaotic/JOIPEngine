#include "EditorActionBar.h"
#include "ui_EditorActionBar.h"

CEditorActionBar::CEditorActionBar(QWidget* pParent) :
  CEditorWidgetBase(pParent),
  m_spUi(std::make_unique<Ui::CEditorActionBar>())
{
  m_spUi->setupUi(this);
}

CEditorActionBar::~CEditorActionBar()
{
}

//----------------------------------------------------------------------------------------
//
void CEditorActionBar::Initialize()
{
  SetInitialized(false);

  HideAllBars();

  SetInitialized(true);
}

//----------------------------------------------------------------------------------------
//
void CEditorActionBar::HideAllBars()
{
  m_spUi->pProjectContainer->hide();
  m_spUi->pResourcesContainer->hide();
}

//----------------------------------------------------------------------------------------
//
void CEditorActionBar::ShowProjectActionBar()
{
  HideAllBars();
  m_spUi->pProjectContainer->show();
}

//----------------------------------------------------------------------------------------
//
void CEditorActionBar::ShowResourceActionBar()
{
  HideAllBars();
  m_spUi->pResourcesContainer->show();
}
