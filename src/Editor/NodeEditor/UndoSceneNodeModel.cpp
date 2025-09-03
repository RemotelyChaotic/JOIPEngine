#include "UndoSceneNodeModel.h"
#include "CommandNodeEdited.h"

CUndoSceneNodeModel::CUndoSceneNodeModel() :
  CSceneNodeModel(),
  CUndoStackAwareModel()
{

}
CUndoSceneNodeModel::~CUndoSceneNodeModel() = default;

//----------------------------------------------------------------------------------------
//
void CUndoSceneNodeModel::SlotCanStartHereChangedImpl(bool bValue)
{
  QJsonObject oldState = save();
  CSceneNodeModel::SlotCanStartHereChangedImpl(bValue);
  QJsonObject newState = save();

  if (nullptr != UndoStack() && !m_bIsInUndoOperation)
  {
    UndoStack()->push(new CCommandNodeEdited(m_pScene, m_id, oldState, newState));
  }
}

//----------------------------------------------------------------------------------------
//
void CUndoSceneNodeModel::SlotNameChangedImpl(const QString& sName)
{
  QJsonObject oldState = save();
  CSceneNodeModel::SlotNameChangedImpl(sName);
  QJsonObject newState = save();

  if (nullptr != UndoStack() && !m_bIsInUndoOperation)
  {
    QJsonObject newState = save();
    UndoStack()->push(new CCommandNodeEdited(m_pScene, m_id, oldState, newState));
  }
}

//----------------------------------------------------------------------------------------
//
void CUndoSceneNodeModel::SlotLayoutChangedImpl(const QString& sName)
{
  QJsonObject oldState = save();
  CSceneNodeModel::SlotLayoutChangedImpl(sName);
  QJsonObject newState = save();

  if (nullptr != UndoStack() && !m_bIsInUndoOperation)
  {
    QJsonObject newState = save();
    UndoStack()->push(new CCommandNodeEdited(m_pScene, m_id, oldState, newState));
  }
}

//----------------------------------------------------------------------------------------
//
void CUndoSceneNodeModel::SlotScriptChangedImpl(const QString& sName)
{
  QJsonObject oldState = save();
  CSceneNodeModel::SlotScriptChangedImpl(sName);
  QJsonObject newState = save();

  if (nullptr != UndoStack() && !m_bIsInUndoOperation)
  {
    QJsonObject newState = save();
    UndoStack()->push(new CCommandNodeEdited(m_pScene, m_id, oldState, newState));
  }
}

//----------------------------------------------------------------------------------------
//
void CUndoSceneNodeModel::SlotTitleResourceChangedImpl(const QString& sOld, const QString& sNew)
{
  QJsonObject oldState = save();
  CSceneNodeModel::SlotTitleResourceChangedImpl(sOld, sNew);
  QJsonObject newState = save();

  if (nullptr != UndoStack() && !m_bIsInUndoOperation)
  {
    QJsonObject newState = save();
    UndoStack()->push(new CCommandNodeEdited(m_pScene, m_id, oldState, newState));
  }
}

//----------------------------------------------------------------------------------------
//
void CUndoSceneNodeModel::UndoRestore(QJsonObject const& p)
{
  m_bIsInUndoOperation = true;
  QJsonValue v = p["sName"];
  if (!v.isUndefined())
  {
    SlotNameChanged(v.toString());
    m_sOldSceneName = m_sSceneName;
  }
  v = p["sScript"];
  if (!v.isUndefined())
  {
    SlotScriptChanged(v.toString());
  }
  v = p["sLayout"];
  if (!v.isUndefined())
  {
    SlotLayoutChanged(v.toString());
  }
  v = p["sTitle"];
  if (!v.isUndefined())
  {
    SlotTitleResourceChanged(QString(), v.toString());
  }
  v = p["bCanStartHere"];
  if (!v.isUndefined())
  {
    SlotCanStartHereChanged(v.toBool());
  }
  m_bIsInUndoOperation = false;
}

//----------------------------------------------------------------------------------------
//
CUndoSceneNodeModelWithWidget::CUndoSceneNodeModelWithWidget()  :
    CSceneNodeModelWithWidget(),
    CUndoStackAwareModel()
{

}
CUndoSceneNodeModelWithWidget::~CUndoSceneNodeModelWithWidget() = default;

//----------------------------------------------------------------------------------------
//
void CUndoSceneNodeModelWithWidget::SlotCanStartHereChangedImpl(bool bValue)
{
  QJsonObject oldState = save();
  CSceneNodeModelWithWidget::SlotCanStartHereChangedImpl(bValue);
  QJsonObject newState = save();

  if (nullptr != UndoStack() && !m_bIsInUndoOperation)
  {
    UndoStack()->push(new CCommandNodeEdited(m_pScene, m_id, oldState, newState));
  }
}

//----------------------------------------------------------------------------------------
//
void CUndoSceneNodeModelWithWidget::SlotNameChangedImpl(const QString& sName)
{
  QJsonObject oldState = save();
  CSceneNodeModelWithWidget::SlotNameChangedImpl(sName);
  QJsonObject newState = save();

  if (nullptr != UndoStack() && !m_bIsInUndoOperation)
  {
    QJsonObject newState = save();
    UndoStack()->push(new CCommandNodeEdited(m_pScene, m_id, oldState, newState));
  }
}

//----------------------------------------------------------------------------------------
//
void CUndoSceneNodeModelWithWidget::SlotLayoutChangedImpl(const QString& sName)
{
  QJsonObject oldState = save();
  CSceneNodeModelWithWidget::SlotLayoutChangedImpl(sName);
  QJsonObject newState = save();

  if (nullptr != UndoStack() && !m_bIsInUndoOperation)
  {
    QJsonObject newState = save();
    UndoStack()->push(new CCommandNodeEdited(m_pScene, m_id, oldState, newState));
  }
}

//----------------------------------------------------------------------------------------
//
void CUndoSceneNodeModelWithWidget::SlotScriptChangedImpl(const QString& sName)
{
  QJsonObject oldState = save();
  CSceneNodeModelWithWidget::SlotScriptChangedImpl(sName);
  QJsonObject newState = save();

  if (nullptr != UndoStack() && !m_bIsInUndoOperation)
  {
    QJsonObject newState = save();
    UndoStack()->push(new CCommandNodeEdited(m_pScene, m_id, oldState, newState));
  }
}

//----------------------------------------------------------------------------------------
//
void CUndoSceneNodeModelWithWidget::SlotTitleResourceChangedImpl(const QString& sOld, const QString& sNew)
{
  QJsonObject oldState = save();
  CSceneNodeModelWithWidget::SlotTitleResourceChangedImpl(sOld, sNew);
  QJsonObject newState = save();

  if (nullptr != UndoStack() && !m_bIsInUndoOperation)
  {
    QJsonObject newState = save();
    UndoStack()->push(new CCommandNodeEdited(m_pScene, m_id, oldState, newState));
  }
}

//----------------------------------------------------------------------------------------
//
void CUndoSceneNodeModelWithWidget::UndoRestore(QJsonObject const& p)
{
  m_bIsInUndoOperation = true;
  QJsonValue v = p["sName"];
  if (!v.isUndefined())
  {
    SlotNameChanged(v.toString());
    m_sOldSceneName = m_sSceneName;
  }
  v = p["sScript"];
  if (!v.isUndefined())
  {
    SlotScriptChanged(v.toString());
  }
  v = p["sLayout"];
  if (!v.isUndefined())
  {
    SlotLayoutChanged(v.toString());
  }
  v = p["sTitle"];
  if (!v.isUndefined())
  {
    SlotTitleResourceChanged(QString(), v.toString());
  }
  v = p["bCanStartHere"];
  if (!v.isUndefined())
  {
    SlotCanStartHereChanged(v.toBool());
  }
  m_bIsInUndoOperation = false;
}

//----------------------------------------------------------------------------------------
//
void CUndoSceneNodeModelWithWidget::OnUndoStackSet()
{
  /*
  if (nullptr != m_pWidget)
  {
    m_pWidget->SetUndoStack(m_pUndoStack);
  }
  */
}
