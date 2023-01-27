#include "CommandInsertEosCommand.h"
#include "Editor/EditorCommandIds.h"
#include "Editor/Script/EosScriptModel.h"

//----------------------------------------------------------------------------------------
//
CCommandInsertEosCommand::CCommandInsertEosCommand(
    QPointer<CEosScriptModel> pModel,
    const SItemIndexPath& path,
    QString sType,
    const tInstructionMapValue& args,
    QUndoCommand* pParent) :
  QUndoCommand(QString("Inserted %1").arg(sType), pParent),
  m_pModel(pModel),
  m_pathCurrent(path),
  m_pathInserted(),
  m_sType(sType),
  m_args(args)
{
}
CCommandInsertEosCommand::~CCommandInsertEosCommand()
{

}

//----------------------------------------------------------------------------------------
//
void CCommandInsertEosCommand::undo()
{
  if (nullptr != m_pModel)
  {
    m_pModel->RemoveInstructionImpl(m_pModel->GetIndex(m_pathInserted));
  }
}

//----------------------------------------------------------------------------------------
//
void CCommandInsertEosCommand::redo()
{
  if (nullptr != m_pModel)
  {
    QModelIndex idx = m_pModel->InsertInstructionImpl(
          m_pModel->GetIndex(m_pathCurrent), m_sType, m_args);
    m_pModel->GetIndexPath(idx, -1, &m_pathInserted);
  }
}

//----------------------------------------------------------------------------------------
//
int CCommandInsertEosCommand::id() const
{
  return EEditorCommandId::eInsertEosCommand;
}

//----------------------------------------------------------------------------------------
//
bool CCommandInsertEosCommand::mergeWith(const QUndoCommand*)
{
  return false;
}
