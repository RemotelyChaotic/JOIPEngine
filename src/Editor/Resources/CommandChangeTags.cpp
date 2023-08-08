#include "CommandChangeTags.h"
#include "Application.h"

#include "Editor/EditorCommandIds.h"

#include "Systems/DatabaseManager.h"

CCommandAddTag::CCommandAddTag(const QString& sProject, const QString& sResource,
                               const tspTag& spTag) :
    QUndoCommand(QObject::tr("Tag %1 added to Resource %2").arg(spTag->m_sName).arg(sResource)),
    m_sCurrentProject(sProject),
    m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
    m_sResource(sResource),
    m_spTag(std::make_shared<STag>(*spTag))
{
  m_spTag->m_spParent = nullptr;
}
CCommandAddTag::~CCommandAddTag() = default;

//----------------------------------------------------------------------------------------
//
void CCommandAddTag::undo()
{
  if (auto spDbManager = m_wpDbManager.lock(); nullptr != spDbManager)
  {
    auto spProject = spDbManager->FindProject(m_sCurrentProject);
    if (nullptr != spProject)
    {
      spDbManager->RemoveTagFromResource(spProject, m_sResource, m_spTag->m_sName);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CCommandAddTag::redo()
{
  if (auto spDbManager = m_wpDbManager.lock(); nullptr != spDbManager)
  {
    auto spProject = spDbManager->FindProject(m_sCurrentProject);
    if (nullptr != spProject)
    {
      auto spTag = spDbManager->FindTagInProject(spProject, m_spTag->m_sName);
      if (nullptr == spTag)
      {
        spDbManager->AddTag(spProject, m_sResource,
                            m_spTag->m_sType, m_spTag->m_sName, m_spTag->m_sDescribtion);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
int CCommandAddTag::id() const
{
  return EEditorCommandId::eAddTag;
}

//----------------------------------------------------------------------------------------
//
bool CCommandAddTag::mergeWith(const QUndoCommand* pOther)
{
  if (pOther->id() != id()) { return false; }
  const CCommandAddTag* pOtherCasted = dynamic_cast<const CCommandAddTag*>(pOther);
  if (m_sCurrentProject != pOtherCasted->m_sCurrentProject ||
      m_sResource != pOtherCasted->m_sResource ||
      m_spTag->m_sName != pOtherCasted->m_spTag->m_sName)
  {
    return false;
  }
  return true;
}

//----------------------------------------------------------------------------------------
//
CCommandRemoveTag::CCommandRemoveTag(const QString& sProject, const QString& sResource,
                                       const tspTag& spTag) :
    QUndoCommand(QObject::tr("Tag %1 removed from Resource %2").arg(spTag->m_sName).arg(sResource)),
    m_sCurrentProject(sProject),
    m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
    m_sResource(sResource),
    m_spTag(std::make_shared<STag>(*spTag))
{
  m_spTag->m_spParent = nullptr;
}
CCommandRemoveTag::~CCommandRemoveTag() = default;

//----------------------------------------------------------------------------------------
//
void CCommandRemoveTag::undo()
{
  if (auto spDbManager = m_wpDbManager.lock(); nullptr != spDbManager)
  {
    auto spProject = spDbManager->FindProject(m_sCurrentProject);
    if (nullptr != spProject)
    {
      auto spTag = spDbManager->FindTagInProject(spProject, m_spTag->m_sName);
      if (nullptr == spTag)
      {
        spDbManager->AddTag(spProject, m_sResource,
                            m_spTag->m_sType, m_spTag->m_sName, m_spTag->m_sDescribtion);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CCommandRemoveTag::redo()
{
  if (auto spDbManager = m_wpDbManager.lock(); nullptr != spDbManager)
  {
    auto spProject = spDbManager->FindProject(m_sCurrentProject);
    if (nullptr != spProject)
    {
      spDbManager->RemoveTagFromResource(spProject, m_sResource, m_spTag->m_sName);
    }
  }
}

//----------------------------------------------------------------------------------------
//
int CCommandRemoveTag::id() const
{
  return EEditorCommandId::eRemoveTag;
}

//----------------------------------------------------------------------------------------
//
bool CCommandRemoveTag::mergeWith(const QUndoCommand* pOther)
{
  if (pOther->id() != id()) { return false; }
  const CCommandRemoveTag* pOtherCasted = dynamic_cast<const CCommandRemoveTag*>(pOther);
  if (m_sCurrentProject != pOtherCasted->m_sCurrentProject ||
      m_sResource != pOtherCasted->m_sResource ||
      m_spTag->m_sName != pOtherCasted->m_spTag->m_sName)
  {
    return false;
  }
  return true;
}
