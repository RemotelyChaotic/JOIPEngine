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



  SetInitialized(true);
}
