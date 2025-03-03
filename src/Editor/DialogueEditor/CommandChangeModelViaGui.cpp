#include "CommandChangeModelViaGui.h"
#include "DialogueEditorTreeModel.h"

#include "Editor/EditorCommandIds.h"

CCommandChangeDialogueModelViaGui::CCommandChangeDialogueModelViaGui(const QVariant& oldValue, const QVariant& newValue,
                                                     const QString& sHeader, const QStringList& vsPath,
                                                     QPointer<CDialogueEditorTreeModel> pModel) :
  QUndoCommand(QString("Changed '%1' value").arg(sHeader)),
  m_pModel(pModel),
  m_vsPath(vsPath),
  m_oldValue(oldValue),
  m_newValue(newValue),
  m_sHeader(sHeader)
{

}
CCommandChangeDialogueModelViaGui::~CCommandChangeDialogueModelViaGui()
{

}

//----------------------------------------------------------------------------------------
//
void CCommandChangeDialogueModelViaGui::undo()
{
  DoUndo(m_oldValue);
}

//----------------------------------------------------------------------------------------
//
void CCommandChangeDialogueModelViaGui::redo()
{
  DoUndo(m_newValue);
}

//----------------------------------------------------------------------------------------
//
int CCommandChangeDialogueModelViaGui::id() const
{
  return EEditorCommandId::eChangeDialoguePropertyViaGui;
}

//----------------------------------------------------------------------------------------
//
bool CCommandChangeDialogueModelViaGui::mergeWith(const QUndoCommand* pOther)
{
  if (nullptr == pOther || id() != pOther->id())
  {
    return false;
  }
  const CCommandChangeDialogueModelViaGui* pOtherCasted = dynamic_cast<const CCommandChangeDialogueModelViaGui*>(pOther);
  if (nullptr == pOtherCasted) { return false; }
  if (m_vsPath != pOtherCasted->m_vsPath || m_sHeader != pOtherCasted->m_sHeader) { return false; }

  m_newValue = pOtherCasted->m_newValue;

  return true;
}

//----------------------------------------------------------------------------------------
//
void CCommandChangeDialogueModelViaGui::DoUndo(const QVariant& val)
{
  if (nullptr != m_pModel)
  {
    QModelIndex idx = m_pModel->Index(m_vsPath);
    qint32 iHeader = -1;
    for (qint32 i = 0; m_pModel->columnCount(idx.parent()) > i; ++i)
    {
      if (m_pModel->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString() == m_sHeader)
      {
        iHeader = i;
      }
    }
    if (-1 != iHeader)
    {
      idx = m_pModel->index(idx.row(), iHeader, idx.parent());
      m_pModel->setData(idx, val, Qt::EditRole);
    }
  }
}
