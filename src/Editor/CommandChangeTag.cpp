#include "CommandChangeTag.h"
#include "Application.h"

#include "Editor/EditorCommandIds.h"

#include "Systems/DatabaseManager.h"
#include "Systems/Tag.h"

//----------------------------------------------------------------------------------------
//
CCommandChangeTag::CCommandChangeTag(const QString& sProject, const QString& sTag,
                                     const QString& sNewCategory, const QString& sOldCategory,
                                     const QString& sNewDescribtion, const QString& sOldDescribtion) :
    QUndoCommand(QObject::tr("Tag %1 changed").arg(sTag)),
    m_sCurrentProject(sProject),
    m_sTag(sTag),
    m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
    m_sNewCategory(sNewCategory),
    m_sOldCategory(sOldCategory),
    m_sNewDescribtion(sNewDescribtion),
    m_sOldDescribtion(sOldDescribtion)
{
}
CCommandChangeTag::~CCommandChangeTag() = default;

//----------------------------------------------------------------------------------------
//
void CCommandChangeTag::undo()
{
  if (auto spDbManager = m_wpDbManager.lock(); nullptr != spDbManager)
  {
    auto spProject = spDbManager->FindProject(m_sCurrentProject);
    if (nullptr != spProject)
    {
      auto spTag = spDbManager->FindTagInProject(spProject, m_sTag);
      if (nullptr != spTag)
      {
        QWriteLocker locker(&spTag->m_rwLock);
        spTag->m_sType = m_sOldCategory;
        spTag->m_sDescribtion = m_sOldDescribtion;
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CCommandChangeTag::redo()
{
  if (auto spDbManager = m_wpDbManager.lock(); nullptr != spDbManager)
  {
    auto spProject = spDbManager->FindProject(m_sCurrentProject);
    if (nullptr != spProject)
    {
      auto spTag = spDbManager->FindTagInProject(spProject, m_sTag);
      if (nullptr != spTag)
      {
        QWriteLocker locker(&spTag->m_rwLock);
        spTag->m_sType = m_sNewCategory;
        spTag->m_sDescribtion = m_sNewDescribtion;
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
int CCommandChangeTag::id() const
{
  return EEditorCommandId::eChangeTag;
}

//----------------------------------------------------------------------------------------
//
bool CCommandChangeTag::mergeWith(const QUndoCommand* pOther)
{
  if (pOther->id() != id()) { return false; }
  const CCommandChangeTag* pOtherCasted = dynamic_cast<const CCommandChangeTag*>(pOther);
  if (m_sCurrentProject != pOtherCasted->m_sCurrentProject ||
      m_sTag != pOtherCasted->m_sTag)
  {
    return false;
  }

  m_sOldCategory = pOtherCasted->m_sOldCategory;
  m_sOldDescribtion = pOtherCasted->m_sOldDescribtion;

  return true;
}
