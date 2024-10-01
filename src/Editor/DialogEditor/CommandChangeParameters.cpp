#include "CommandChangeParameters.h"
#include "DialogEditorTreeModel.h"

#include "Editor/EditorCommandIds.h"


CCommandChangeParameters::CCommandChangeParameters(const tspProject& spProject,
                                                   const QStringList& vsPath,
                                                   std::shared_ptr<CDialogNode> spNode,
                                                   QPointer<CDialogEditorTreeModel> pModel,
                                                   QUndoCommand* pParent) :
  QUndoCommand("Changed dialog parameters: " + (nullptr != spNode && !spNode->m_sName.isEmpty() ? spNode->m_sName : "<empty>"), pParent),
  m_spProject(spProject),
  m_pModel(pModel),
  m_vsPathNew(vsPath)
{
  if (nullptr != pModel)
  {
    auto oldNode = pModel->Node(pModel->Index(vsPath));
    m_nodeOld = dialog_tree::SerializeNode(oldNode);
    m_vsPathOld = m_vsPathNew;
    m_vsPathOld.erase(m_vsPathOld.begin()+m_vsPathOld.size()-1);
    m_vsPathOld.push_back(spNode->m_sName);
  }
  m_nodeNew = dialog_tree::SerializeNode(spNode);
}

CCommandChangeParameters::~CCommandChangeParameters()
{

}

//----------------------------------------------------------------------------------------
//
void CCommandChangeParameters::undo()
{
  DoUndoRedo(m_nodeOld, m_vsPathOld);
}

//----------------------------------------------------------------------------------------
//
void CCommandChangeParameters::redo()
{
  DoUndoRedo(m_nodeNew, m_vsPathNew);
}

//----------------------------------------------------------------------------------------
//
int CCommandChangeParameters::id() const
{
  return EEditorCommandId::eChangeDialogParams;
}

//----------------------------------------------------------------------------------------
//
bool CCommandChangeParameters::mergeWith(const QUndoCommand* pOther)
{
  if (nullptr == pOther || pOther->id() != id())
  {
    return false;
  }
  auto pOtherCasted = dynamic_cast<const CCommandChangeParameters*>(pOther);
  if (nullptr == pOtherCasted)
  {
    return false;
  }

  m_nodeNew = pOtherCasted->m_nodeNew;
  m_vsPathOld = pOtherCasted->m_vsPathOld;
  m_vsPathNew = pOtherCasted->m_vsPathNew;
  return true;
}

//----------------------------------------------------------------------------------------
//
void CCommandChangeParameters::DoUndoRedo(const QByteArray& arr, const QStringList& vsPath)
{
  if (nullptr != m_pModel)
  {
    QModelIndex idx = m_pModel->Index(vsPath);
    auto spNodeDeser = dialog_tree::DeserializeNode(arr, m_spProject);
    m_pModel->UpdateFrom(idx, spNodeDeser);
  }
}
