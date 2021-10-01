#include "EditorWidgetBase.h"
#include "Editor/EditorActionBar.h"
#include "Editor/EditorModel.h"
#include "Editor/NodeEditor/FlowScene.h"
#include "Editor/Project/KinkTreeModel.h"
#include "Editor/Resources/ResourceTreeItemModel.h"
#include "Editor/Script/ScriptEditorModel.h"
#include <QUndoStack>

CEditorWidgetBase::CEditorWidgetBase(QWidget* pParent) :
  QWidget(pParent),
  IWidgetBaseInterface(),
  m_pOriginalParent(pParent),
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
void CEditorWidgetBase::TakeFromLayout()
{
  setParent(m_pOriginalParent);
}

//----------------------------------------------------------------------------------------
//
QPointer<CEditorActionBar> CEditorWidgetBase::ActionBar() const
{
  return m_pActionBar;
}

//----------------------------------------------------------------------------------------
//
QPointer<CEditorModel> CEditorWidgetBase::EditorModel() const
{
  return m_pEditorModel;
}

//----------------------------------------------------------------------------------------
//
QPointer<CFlowScene> CEditorWidgetBase::FlowSceneModel() const
{
  return m_pEditorModel->FlowSceneModel();
}

//----------------------------------------------------------------------------------------
//
QPointer<CKinkTreeModel> CEditorWidgetBase::KinkModel() const
{
  return  m_pEditorModel->KinkTreeModel();
}

//----------------------------------------------------------------------------------------
//
QPointer<CResourceTreeItemModel> CEditorWidgetBase::ResourceTreeModel() const
{
  return m_pEditorModel->ResourceTreeModel();
}

//----------------------------------------------------------------------------------------
//
QPointer<CScriptEditorModel> CEditorWidgetBase::ScriptEditorModel() const
{
  return m_pEditorModel->ScriptEditorModel();
}

//----------------------------------------------------------------------------------------
//
QPointer<QUndoStack> CEditorWidgetBase::UndoStack() const
{
  return m_pEditorModel->UndoStack();
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
