#include "CommandAddRemoveNode.h"
#include "DialogueEditorTreeModel.h"

#include "Editor/EditorCommandIds.h"

namespace
{
  void RemoveNode(QPointer<CDialogueEditorTreeModel> pModel,
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
                  QPointer<CDialogueEditorTreeModel> pModel,
                  qint32 iIndex,
                  QStringList& vsPath,
                  const QByteArray& node)
  {
    if (nullptr != pModel)
    {
      QStringList vsMidList = vsPath.mid(0, vsPath.length()-1);
      auto spNode = dialogue_tree::DeserializeNode(node, spProject);
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
CCommandAddDialogueNode::CCommandAddDialogueNode(const tspProject& spProject,
                                             const QStringList& vsPath,
                                             qint32 iIndex,
                                             std::shared_ptr<CDialogueNode> spNode,
                                             QPointer<CDialogueEditorTreeModel> pModel,
                                             QUndoCommand* pParent) :
  QUndoCommand("Added dialog node: " + (nullptr != spNode && !spNode->m_sName.isEmpty() ? spNode->m_sName : "<empty>"), pParent),
  m_spProject(spProject),
  m_pModel(pModel),
  m_vsPath(vsPath),
  m_iPos(iIndex)
{
  m_node = dialogue_tree::SerializeNode(spNode);
}
CCommandAddDialogueNode::~CCommandAddDialogueNode()
{
}

//----------------------------------------------------------------------------------------
//
void CCommandAddDialogueNode::undo()
{
  RemoveNode(m_pModel, m_vsPath);
}

//----------------------------------------------------------------------------------------
//
void CCommandAddDialogueNode::redo()
{
  InsertNode(m_spProject, m_pModel, m_iPos, m_vsPath, m_node);
}

//----------------------------------------------------------------------------------------
//
int CCommandAddDialogueNode::id() const
{
  return EEditorCommandId::eAddDialogueNode;
}

//----------------------------------------------------------------------------------------
//
bool CCommandAddDialogueNode::mergeWith(const QUndoCommand* pOther)
{
  Q_UNUSED(pOther)
  return false;
}

//----------------------------------------------------------------------------------------
//
CCommandRemoveDialogueNode::CCommandRemoveDialogueNode(const tspProject& spProject,
                                                   const QStringList& vsPath,
                                                   QPointer<CDialogueEditorTreeModel> pModel,
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
      m_node = dialogue_tree::SerializeNode(spNode);
      m_iPos = idx.row();
    }
  }
}
CCommandRemoveDialogueNode::~CCommandRemoveDialogueNode()
{
}

//----------------------------------------------------------------------------------------
//
void CCommandRemoveDialogueNode::undo()
{
  InsertNode(m_spProject, m_pModel, m_iPos, m_vsPath, m_node);
}

//----------------------------------------------------------------------------------------
//
void CCommandRemoveDialogueNode::redo()
{
  RemoveNode(m_pModel, m_vsPath);
}

//----------------------------------------------------------------------------------------
//
int CCommandRemoveDialogueNode::id() const
{
  return EEditorCommandId::eAddDialogueNode;
}

//----------------------------------------------------------------------------------------
//
bool CCommandRemoveDialogueNode::mergeWith(const QUndoCommand* pOther)
{
  Q_UNUSED(pOther)
  return false;
}
