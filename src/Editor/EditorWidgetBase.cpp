#include "EditorWidgetBase.h"
#include "EditorModel.h"
#include "Resources/ResourceTreeItemModel.h"

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
void CEditorWidgetBase::SetEditorModel(CEditorModel* pItemModel)
{
  m_pEditorModel = pItemModel;
}

//----------------------------------------------------------------------------------------
//
CEditorActionBar* CEditorWidgetBase::ActionBar() const
{
  return m_pActionBar;
}

//----------------------------------------------------------------------------------------
//
CEditorModel* CEditorWidgetBase::EditorModel() const
{
  return m_pEditorModel;
}

//----------------------------------------------------------------------------------------
//
CResourceTreeItemModel* CEditorWidgetBase::ResourceTreeModel() const
{
  return m_pEditorModel->ResourceTreeModel();
}
