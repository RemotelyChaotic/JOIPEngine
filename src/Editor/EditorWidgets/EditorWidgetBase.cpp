#include "EditorWidgetBase.h"

#include "Editor/EditorActionBar.h"
#include "Editor/EditorModel.h"
#include "Editor/EditorEditableFileModel.h"

#include "Editor/DialogEditor/DialogEditorTreeModel.h"
#include "Editor/NodeEditor/FlowScene.h"
#include "Editor/Project/KinkTreeModel.h"
#include "Editor/Resources/ResourceTreeItemModel.h"

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
void CEditorWidgetBase::SetIcon(const QIcon& icon)
{
  m_icon = icon;
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
QPointer<CDialogEditorTreeModel> CEditorWidgetBase::DialogModel() const
{
  return m_pEditorModel->DialogModel();
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
QPointer<CEditorEditableFileModel> CEditorWidgetBase::EditableFileModel() const
{
  return m_pEditorModel->EditableFileModel();
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
