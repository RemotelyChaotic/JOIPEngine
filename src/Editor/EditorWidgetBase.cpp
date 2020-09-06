#include "EditorWidgetBase.h"
#include "EditorModel.h"
#include "Resources/ResourceTreeItemModel.h"

CEditorWidgetBase::CEditorWidgetBase(QWidget* pParent) :
  QWidget(pParent),
  IWidgetBaseInterface(),
  m_pActionBar(nullptr),
  m_bLoaded(false)
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
QtNodes::FlowScene* CEditorWidgetBase::FlowSceneModel() const
{
  return m_pEditorModel->FlowSceneModel();
}

//----------------------------------------------------------------------------------------
//
CKinkTreeModel* CEditorWidgetBase::KinkModel() const
{
  return  m_pEditorModel->KinkTreeModel();
}

//----------------------------------------------------------------------------------------
//
CResourceTreeItemModel* CEditorWidgetBase::ResourceTreeModel() const
{
  return m_pEditorModel->ResourceTreeModel();
}

//----------------------------------------------------------------------------------------
//
CScriptEditorModel* CEditorWidgetBase::ScriptEditorModel() const
{
  return m_pEditorModel->ScriptEditorModel();
}

//----------------------------------------------------------------------------------------
//
void CEditorWidgetBase::SetLoaded(bool bLoaded)
{
  if (m_bLoaded != bLoaded)
  {
    m_bLoaded = bLoaded;
    if (!bLoaded)
    {
      emit SignalUnloadFinished();
    }
  }
}
