#include "CommandAddRemoveNode.h"
#include "DialogEditorTreeModel.h"

#include "Editor/EditorCommandIds.h"

namespace
{
  void RemoveNode(QPointer<CDialogEditorTreeModel> pModel,
                  const QStringList& vsPath)
  {
    if (nullptr != pModel)
    {
      QModelIndex idx = pModel->Index(vsPath);
      pModel->removeRow(idx.row(), idx.parent());
    }
  }

  //--------------------------------------------------------------------------------------
  //
  void InsertNode(tspProject spProject,
                  QPointer<CDialogEditorTreeModel> pModel,
                  qint32 iIndex,
                  QStringList& vsPath,
                  const QByteArray& node)
  {
    if (nullptr != pModel)
    {
      QStringList vsMidList = vsPath.mid(0, vsPath.length()-1);
      auto spNode = dialog_tree::DeserializeNode(node, spProject);
      QModelIndex idx = pModel->Index(vsMidList);
      bool bOk = pModel->insertNode(iIndex, spNode, idx);
      if (bOk)
      {
        idx = pModel->Index(vsMidList);
        vsPath = pModel->Path(pModel->index(iIndex, idx.column(), idx));
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
CCommandAddDialogNode::CCommandAddDialogNode(const tspProject& spProject,
                                             const QStringList& vsPath,
                                             qint32 iIndex,
                                             std::shared_ptr<CDialogNode> spNode,
                                             QPointer<CDialogEditorTreeModel> pModel,
                                             QUndoCommand* pParent) :
  QUndoCommand("Added dialog node: " + (nullptr != spNode && !spNode->m_sName.isEmpty() ? spNode->m_sName : "<empty>"), pParent),
  m_spProject(spProject),
  m_pModel(pModel),
  m_vsPath(vsPath),
  m_iPos(iIndex)
{
  m_node = dialog_tree::SerializeNode(spNode);
}
CCommandAddDialogNode::~CCommandAddDialogNode()
{
}

//----------------------------------------------------------------------------------------
//
void CCommandAddDialogNode::undo()
{
  RemoveNode(m_pModel, m_vsPath);
}

//----------------------------------------------------------------------------------------
//
void CCommandAddDialogNode::redo()
{
  InsertNode(m_spProject, m_pModel, m_iPos, m_vsPath, m_node);
}

//----------------------------------------------------------------------------------------
//
int CCommandAddDialogNode::id() const
{
  return EEditorCommandId::eAddDialogNode;
}

//----------------------------------------------------------------------------------------
//
bool CCommandAddDialogNode::mergeWith(const QUndoCommand* pOther)
{
  Q_UNUSED(pOther)
  return false;
}

//----------------------------------------------------------------------------------------
//
CCommandRemoveDialogNode::CCommandRemoveDialogNode(const tspProject& spProject,
                                                   const QStringList& vsPath,
                                                   QPointer<CDialogEditorTreeModel> pModel,
                                                   QUndoCommand* pParent) :
  QUndoCommand(pParent),
  m_spProject(spProject),
  m_pModel(pModel),
  m_vsPath(vsPath)
{
  if (nullptr != pModel)
  {
    QModelIndex idx = pModel->Index(vsPath);
    auto spNode = pModel->Node(idx);
    if (nullptr != spNode)
    {
      setText("Removed dialog node: " + (nullptr != spNode && !spNode->m_sName.isEmpty() ? spNode->m_sName : "<empty>"));
      m_node = dialog_tree::SerializeNode(spNode);
      m_iPos = idx.row();
    }
  }
}
CCommandRemoveDialogNode::~CCommandRemoveDialogNode()
{
}

//----------------------------------------------------------------------------------------
//
void CCommandRemoveDialogNode::undo()
{
  InsertNode(m_spProject, m_pModel, m_iPos, m_vsPath, m_node);
}

//----------------------------------------------------------------------------------------
//
void CCommandRemoveDialogNode::redo()
{
  RemoveNode(m_pModel, m_vsPath);
}

//----------------------------------------------------------------------------------------
//
int CCommandRemoveDialogNode::id() const
{
  return EEditorCommandId::eAddDialogNode;
}

//----------------------------------------------------------------------------------------
//
bool CCommandRemoveDialogNode::mergeWith(const QUndoCommand* pOther)
{
  Q_UNUSED(pOther)
  return false;
}
