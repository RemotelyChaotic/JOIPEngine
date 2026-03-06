#include "UndoSubflowNodeModel.h"
#include "CommandNodeEdited.h"

CUndoSubflowNodeModel::CUndoSubflowNodeModel() :
  CSubflowNodeModel(),
  CUndoStackAwareModel()
{
}
CUndoSubflowNodeModel::~CUndoSubflowNodeModel() = default;

//----------------------------------------------------------------------------------------
//
void CUndoSubflowNodeModel::SlotFlowChangedImpl(const QString& sName)
{
  QJsonObject oldState = save();
  CSubflowNodeModel::SlotFlowChangedImpl(sName);
  QJsonObject newState = save();

  if (nullptr != UndoStack() && !m_bIsInUndoOperation)
  {
    UndoStack()->push(new CCommandNodeEdited(m_pScene, m_id, oldState, newState));
  }
}

//----------------------------------------------------------------------------------------
//
void CUndoSubflowNodeModel::SlotNameChangedImpl(const QString& sName)
{
  QJsonObject oldState = save();
  CSubflowNodeModel::SlotNameChangedImpl(sName);
  QJsonObject newState = save();

  if (nullptr != UndoStack() && !m_bIsInUndoOperation)
  {
    QJsonObject newState = save();
    UndoStack()->push(new CCommandNodeEdited(m_pScene, m_id, oldState, newState));
  }
}

//----------------------------------------------------------------------------------------
//
void CUndoSubflowNodeModel::UndoRestore(QJsonObject const& p)
{
  m_bIsInUndoOperation = true;
  QJsonValue v = p["sName"];
  if (!v.isUndefined())
  {
    SlotNameChanged(v.toString());
  }
  v = p["sFlow"];
  if (!v.isUndefined())
  {
    SlotFlowChanged(v.toString());
  }
  m_bIsInUndoOperation = false;
}

//----------------------------------------------------------------------------------------
//
CUndoSubflowNodeModelWithWidget::CUndoSubflowNodeModelWithWidget()  :
    CSubflowNodeModelWithWidget(),
    CUndoStackAwareModel()
{
}
CUndoSubflowNodeModelWithWidget::~CUndoSubflowNodeModelWithWidget() = default;

//----------------------------------------------------------------------------------------
//
void CUndoSubflowNodeModelWithWidget::SlotFlowChangedImpl(const QString& sName)
{
  QJsonObject oldState = save();
  CSubflowNodeModelWithWidget::SlotFlowChangedImpl(sName);
  QJsonObject newState = save();

  if (nullptr != UndoStack() && !m_bIsInUndoOperation)
  {
    QJsonObject newState = save();
    UndoStack()->push(new CCommandNodeEdited(m_pScene, m_id, oldState, newState));
  }
}


//----------------------------------------------------------------------------------------
//
void CUndoSubflowNodeModelWithWidget::SlotNameChangedImpl(const QString& sName)
{
  QJsonObject oldState = save();
  CSubflowNodeModelWithWidget::SlotNameChangedImpl(sName);
  QJsonObject newState = save();

  if (nullptr != UndoStack() && !m_bIsInUndoOperation)
  {
    QJsonObject newState = save();
    UndoStack()->push(new CCommandNodeEdited(m_pScene, m_id, oldState, newState));
  }
}

//----------------------------------------------------------------------------------------
//
void CUndoSubflowNodeModelWithWidget::UndoRestore(QJsonObject const& p)
{
  m_bIsInUndoOperation = true;
  QJsonValue v = p["sName"];
  if (!v.isUndefined())
  {
    SlotNameChanged(v.toString());
  }
  v = p["sFlow"];
  if (!v.isUndefined())
  {
    SlotFlowChanged(v.toString());
  }
  m_bIsInUndoOperation = false;
}
