#include "EditorWidgetBase.h"

CEditorWidgetBase::CEditorWidgetBase(QWidget* pParent) :
  QWidget(pParent),
  m_pActionBar(nullptr),
  m_bInitialized(false)
{

}

CEditorWidgetBase::~CEditorWidgetBase()
{

}

//----------------------------------------------------------------------------------------
//
void CEditorWidgetBase::SetActionBar(CEditorActionBar* pActionBar)
{
  OnActionBarAboutToChange();
  m_pActionBar = pActionBar;
  OnActionBarChanged();
}

//----------------------------------------------------------------------------------------
//
CEditorActionBar* CEditorWidgetBase::ActionBar()
{
  return m_pActionBar;
}
