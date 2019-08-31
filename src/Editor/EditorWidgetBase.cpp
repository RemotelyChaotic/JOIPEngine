#include "EditorWidgetBase.h"

CEditorWidgetBase::CEditorWidgetBase(QWidget* pParent) :
  QWidget(pParent),
  IWidgetBaseInterface(),
  m_pActionBar(nullptr)
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
