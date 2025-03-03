#include "CommandAddRemoveDialogueTags.h"
#include "Application.h"

#include "Editor/EditorCommandIds.h"
#include "Editor/DialogueEditor/DialogueEditorTreeModel.h"

#include "Systems/DatabaseManager.h"

CCommandAddDialogueTag::CCommandAddDialogueTag(const QString& sProject, const QStringList& vsPath,
                                           std::shared_ptr<CDialogueNode> spNode,
                                           const tspTag& spTag,
                                           QPointer<CDialogueEditorTreeModel> pModel) :
  QUndoCommand(QObject::tr("Tag %1 added to Dialogue %2").arg(spTag->m_sName).arg(spNode->m_sName)),
  m_sCurrentProject(sProject),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_pModel(pModel),
  m_spTag(std::make_shared<STag>(*spTag)),
  m_vsPath(vsPath)
{
  m_spTag->m_spParent = nullptr;
}
CCommandAddDialogueTag::~CCommandAddDialogueTag() = default;

//----------------------------------------------------------------------------------------
//
void CCommandAddDialogueTag::undo()
{
  if (auto spDbManager = m_wpDbManager.lock(); nullptr != spDbManager && nullptr != m_pModel)
  {
    QModelIndex idx = m_pModel->Index(m_vsPath);
    auto spNode = m_pModel->Node(idx);
    auto spDialogNode = std::dynamic_pointer_cast<CDialogueNodeDialogue>(spNode);
    if (nullptr != spDialogNode)
    {
      QReadLocker l(&m_spTag->m_rwLock);
      auto it = spDialogNode->m_tags.find(m_spTag->m_sName);
      if (spDialogNode->m_tags.end() != it)
      {
        spDialogNode->m_tags.erase(it);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CCommandAddDialogueTag::redo()
{
  if (auto spDbManager = m_wpDbManager.lock(); nullptr != spDbManager && nullptr != m_pModel)
  {
    QModelIndex idx = m_pModel->Index(m_vsPath);
    auto spNode = m_pModel->Node(idx);
    auto spDialogNode = std::dynamic_pointer_cast<CDialogueNodeDialogue>(spNode);
    if (nullptr != spDialogNode)
    {
      {
        QReadLocker l(&m_spTag->m_rwLock);
        auto it = spDialogNode->m_tags.find(m_spTag->m_sName);
        if (spDialogNode->m_tags.end() == it)
        {
          spDialogNode->m_tags.insert({m_spTag->m_sName, m_spTag});
        }
      }

      auto spProject = spDbManager->FindProject(m_sCurrentProject);
      if (nullptr != spProject)
      {
        spDbManager->AddTag(spProject, spDialogNode->m_sFileId,
                            m_spTag->m_sType, m_spTag->m_sName, m_spTag->m_sDescribtion);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
int CCommandAddDialogueTag::id() const
{
  return EEditorCommandId::eAddDialogueTags;
}

//----------------------------------------------------------------------------------------
//
bool CCommandAddDialogueTag::mergeWith(const QUndoCommand* pOther)
{
  if (pOther->id() != id()) { return false; }
  const CCommandAddDialogueTag* pOtherCasted = dynamic_cast<const CCommandAddDialogueTag*>(pOther);
  if (m_sCurrentProject != pOtherCasted->m_sCurrentProject ||
      m_vsPath != pOtherCasted->m_vsPath ||
      m_spTag->m_sName != pOtherCasted->m_spTag->m_sName)
  {
    return false;
  }
  return true;
}

//----------------------------------------------------------------------------------------
//
CCommandRemoveDialogueTag::CCommandRemoveDialogueTag(const QString& sProject, const QStringList& vsPath,
                                                 std::shared_ptr<CDialogueNode> spNode,
                                                 const tspTag& spTag,
                                                 QPointer<CDialogueEditorTreeModel> pModel) :
  QUndoCommand(QObject::tr("Tag %1 removed from Dialogue %2").arg(spTag->m_sName).arg(spNode->m_sName)),
  m_sCurrentProject(sProject),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_pModel(pModel),
  m_spTag(std::make_shared<STag>(*spTag)),
  m_vsPath(vsPath)
{
  m_spTag->m_spParent = nullptr;
}
CCommandRemoveDialogueTag::~CCommandRemoveDialogueTag() = default;

//----------------------------------------------------------------------------------------
//
void CCommandRemoveDialogueTag::undo()
{
  if (auto spDbManager = m_wpDbManager.lock(); nullptr != spDbManager && nullptr != m_pModel)
  {
    QModelIndex idx = m_pModel->Index(m_vsPath);
    auto spNode = m_pModel->Node(idx);
    auto spDialogNode = std::dynamic_pointer_cast<CDialogueNodeDialogue>(spNode);
    if (nullptr != spDialogNode)
    {
      {
        QReadLocker l(&m_spTag->m_rwLock);
        auto it = spDialogNode->m_tags.find(m_spTag->m_sName);
        if (spDialogNode->m_tags.end() == it)
        {
          spDialogNode->m_tags.insert({m_spTag->m_sName, m_spTag});
        }
      }

      auto spProject = spDbManager->FindProject(m_sCurrentProject);
      if (nullptr != spProject)
      {
        spDbManager->AddTag(spProject, spDialogNode->m_sFileId,
                            m_spTag->m_sType, m_spTag->m_sName, m_spTag->m_sDescribtion);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CCommandRemoveDialogueTag::redo()
{
  if (auto spDbManager = m_wpDbManager.lock(); nullptr != spDbManager && nullptr != m_pModel)
  {
    QModelIndex idx = m_pModel->Index(m_vsPath);
    auto spNode = m_pModel->Node(idx);
    auto spDialogNode = std::dynamic_pointer_cast<CDialogueNodeDialogue>(spNode);
    if (nullptr != spDialogNode)
    {
      QReadLocker l(&m_spTag->m_rwLock);
      auto it = spDialogNode->m_tags.find(m_spTag->m_sName);
      if (spDialogNode->m_tags.end() != it)
      {
        spDialogNode->m_tags.erase(it);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
int CCommandRemoveDialogueTag::id() const
{
  return EEditorCommandId::eRemoveDialogueTags;
}

//----------------------------------------------------------------------------------------
//
bool CCommandRemoveDialogueTag::mergeWith(const QUndoCommand* pOther)
{
  if (pOther->id() != id()) { return false; }
  const CCommandRemoveDialogueTag* pOtherCasted = dynamic_cast<const CCommandRemoveDialogueTag*>(pOther);
  if (m_sCurrentProject != pOtherCasted->m_sCurrentProject ||
      m_vsPath != pOtherCasted->m_vsPath ||
      m_spTag->m_sName != pOtherCasted->m_spTag->m_sName)
  {
    return false;
  }
  return true;
}
