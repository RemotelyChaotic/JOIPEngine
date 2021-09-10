#include "CommandChangeTitleCard.h"
#include "Application.h"
#include "Editor/EditorCommandIds.h"
#include "Systems/DatabaseManager.h"

CCommandChangeTitleCard::CCommandChangeTitleCard(const tspProject& spCurrentProject,
                                                 const QString& sOldTitleCard,
                                                 const QString& sNewTitleCard,
                                                 const std::function<void(void)>& fnOnChanged,
                                                 QUndoCommand* pParent) :
  QUndoCommand("Title card -> " + sNewTitleCard, pParent),
  m_spCurrentProject(spCurrentProject),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_sOldTitleCard(sOldTitleCard),
  m_sNewTitleCard(sNewTitleCard),
  m_fnOnChanged(fnOnChanged)
{
}

CCommandChangeTitleCard::~CCommandChangeTitleCard()
{}

//----------------------------------------------------------------------------------------
//
void CCommandChangeTitleCard::undo()
{
  DoUndoRedo(m_sOldTitleCard);
}

//----------------------------------------------------------------------------------------
//
void CCommandChangeTitleCard::redo()
{
  DoUndoRedo(m_sNewTitleCard);
}

//----------------------------------------------------------------------------------------
//
int CCommandChangeTitleCard::id() const
{
  return EEditorCommandId::eChangeTitleCard;
}

//----------------------------------------------------------------------------------------
//
bool CCommandChangeTitleCard::mergeWith(const QUndoCommand* pOther)
{
  if (nullptr == pOther || id() != pOther->id())
  {
    return false;
  }

  const CCommandChangeTitleCard* pOtherCasted = dynamic_cast<const CCommandChangeTitleCard*>(pOther);
  if (nullptr == pOtherCasted) { return false; }

  m_sNewTitleCard = pOtherCasted->m_sNewTitleCard;
  setText("Title card -> " + m_sNewTitleCard);
  return true;
}

//----------------------------------------------------------------------------------------
//
void CCommandChangeTitleCard::DoUndoRedo(const QString& sTitleCard)
{
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager && nullptr != m_spCurrentProject)
  {
    tspResource spResource = spDbManager->FindResourceInProject(m_spCurrentProject, sTitleCard);
    if (nullptr != spResource)
    {
      {
        QWriteLocker locker(&m_spCurrentProject->m_rwLock);
        m_spCurrentProject->m_sTitleCard = sTitleCard;
      }

      if (nullptr != m_fnOnChanged) { m_fnOnChanged(); }
    }
  }
}
