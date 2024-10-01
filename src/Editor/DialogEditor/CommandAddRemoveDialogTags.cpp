#include "CommandAddRemoveDialogTags.h"
#include "Application.h"

#include "Editor/EditorCommandIds.h"
#include "Editor/DialogEditor/DialogEditorTreeModel.h"

#include "Systems/DatabaseManager.h"

CCommandAddDialogTag::CCommandAddDialogTag(const QString& sProject, const QStringList& vsPath,
                                           std::shared_ptr<CDialogNode> spNode,
                                           const tspTag& spTag,
                                           QPointer<CDialogEditorTreeModel> pModel) :
  QUndoCommand(QObject::tr("Tag %1 added to Dialog %2").arg(spTag->m_sName).arg(spNode->m_sName)),
  m_sCurrentProject(sProject),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_pModel(pModel),
  m_spTag(std::make_shared<STag>(*spTag)),
  m_vsPath(vsPath)
{
  m_spTag->m_spParent = nullptr;
}
CCommandAddDialogTag::~CCommandAddDialogTag() = default;

//----------------------------------------------------------------------------------------
//
void CCommandAddDialogTag::undo()
{
  if (auto spDbManager = m_wpDbManager.lock(); nullptr != spDbManager && nullptr != m_pModel)
  {
    QModelIndex idx = m_pModel->Index(m_vsPath);
    auto spNode = m_pModel->Node(idx);
    auto spDialogNode = std::dynamic_pointer_cast<CDialogNodeDialog>(spNode);
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
void CCommandAddDialogTag::redo()
{
  if (auto spDbManager = m_wpDbManager.lock(); nullptr != spDbManager && nullptr != m_pModel)
  {
    QModelIndex idx = m_pModel->Index(m_vsPath);
    auto spNode = m_pModel->Node(idx);
    auto spDialogNode = std::dynamic_pointer_cast<CDialogNodeDialog>(spNode);
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
int CCommandAddDialogTag::id() const
{
  return EEditorCommandId::eAddDialogTags;
}

//----------------------------------------------------------------------------------------
//
bool CCommandAddDialogTag::mergeWith(const QUndoCommand* pOther)
{
  if (pOther->id() != id()) { return false; }
  const CCommandAddDialogTag* pOtherCasted = dynamic_cast<const CCommandAddDialogTag*>(pOther);
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
CCommandRemoveDialogTag::CCommandRemoveDialogTag(const QString& sProject, const QStringList& vsPath,
                                                 std::shared_ptr<CDialogNode> spNode,
                                                 const tspTag& spTag,
                                                 QPointer<CDialogEditorTreeModel> pModel) :
  QUndoCommand(QObject::tr("Tag %1 removed from Dialog %2").arg(spTag->m_sName).arg(spNode->m_sName)),
  m_sCurrentProject(sProject),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_pModel(pModel),
  m_spTag(std::make_shared<STag>(*spTag)),
  m_vsPath(vsPath)
{
  m_spTag->m_spParent = nullptr;
}
CCommandRemoveDialogTag::~CCommandRemoveDialogTag() = default;

//----------------------------------------------------------------------------------------
//
void CCommandRemoveDialogTag::undo()
{
  if (auto spDbManager = m_wpDbManager.lock(); nullptr != spDbManager && nullptr != m_pModel)
  {
    QModelIndex idx = m_pModel->Index(m_vsPath);
    auto spNode = m_pModel->Node(idx);
    auto spDialogNode = std::dynamic_pointer_cast<CDialogNodeDialog>(spNode);
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
void CCommandRemoveDialogTag::redo()
{
  if (auto spDbManager = m_wpDbManager.lock(); nullptr != spDbManager && nullptr != m_pModel)
  {
    QModelIndex idx = m_pModel->Index(m_vsPath);
    auto spNode = m_pModel->Node(idx);
    auto spDialogNode = std::dynamic_pointer_cast<CDialogNodeDialog>(spNode);
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
int CCommandRemoveDialogTag::id() const
{
  return EEditorCommandId::eRemoveDialogTags;
}

//----------------------------------------------------------------------------------------
//
bool CCommandRemoveDialogTag::mergeWith(const QUndoCommand* pOther)
{
  if (pOther->id() != id()) { return false; }
  const CCommandRemoveDialogTag* pOtherCasted = dynamic_cast<const CCommandRemoveDialogTag*>(pOther);
  if (m_sCurrentProject != pOtherCasted->m_sCurrentProject ||
      m_vsPath != pOtherCasted->m_vsPath ||
      m_spTag->m_sName != pOtherCasted->m_spTag->m_sName)
  {
    return false;
  }
  return true;
}
