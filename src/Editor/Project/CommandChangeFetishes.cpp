#include "CommandChangeFetishes.h"
#include "Editor/EditorCommandIds.h"
#include "Editor/EditorWidgetTypes.h"

//----------------------------------------------------------------------------------------
//
CCommandAddFetishes::CCommandAddFetishes(QPointer<QWidget> pGuard,
                                         std::function<void(QStringList)> fnAdd,
                                         std::function<void(QStringList)> fnRemove,
                                         QStringList vsKinks,
                                         QUndoCommand* pParent) :
  QUndoCommand(QString("Added %1 kinks").arg(vsKinks.size()), pParent),
  m_pGuard(pGuard),
  m_fnAdd(fnAdd),
  m_fnRemove(fnRemove),
  m_vsAddedKinks(vsKinks)
{
}
CCommandAddFetishes::~CCommandAddFetishes()
{
}

//----------------------------------------------------------------------------------------
//
void CCommandAddFetishes::undo()
{
  if (!m_pGuard.isNull())
  {
    m_fnRemove(m_vsAddedKinks);
  }
}

//----------------------------------------------------------------------------------------
//
void CCommandAddFetishes::redo()
{
  if (!m_pGuard.isNull())
  {
    m_fnAdd(m_vsAddedKinks);
  }
}

//----------------------------------------------------------------------------------------
//
int CCommandAddFetishes::id() const
{
  return EEditorCommandId::eAddFetishes;
}

//----------------------------------------------------------------------------------------
//
bool CCommandAddFetishes::mergeWith(const QUndoCommand* pOther)
{
  if (nullptr == pOther || id() != pOther->id())
  {
    return false;
  }

  const CCommandAddFetishes* pOtherCasted = dynamic_cast<const CCommandAddFetishes*>(pOther);
  if (nullptr == pOtherCasted) { return false; }

  m_vsAddedKinks << pOtherCasted->m_vsAddedKinks;
  setText(QString("Added %1 kinks").arg(m_vsAddedKinks.size()));
  return true;
}

//----------------------------------------------------------------------------------------
//
CCommandRemoveFetishes::CCommandRemoveFetishes(QPointer<QWidget> pGuard,
                                               std::function<void(QStringList)> fnAdd,
                                               std::function<void(QStringList)> fnRemove,
                                               QStringList vsKinks,
                                               QUndoCommand* pParent) :
  QUndoCommand(QString("Removed %1 kinks").arg(vsKinks.size()), pParent),
  m_pGuard(pGuard),
  m_fnAdd(fnAdd),
  m_fnRemove(fnRemove),
  m_vsRemovedKinks(vsKinks)
{

}
CCommandRemoveFetishes::~CCommandRemoveFetishes()
{

}

//----------------------------------------------------------------------------------------
//
void CCommandRemoveFetishes::undo()
{
  if (!m_pGuard.isNull())
  {
    m_fnRemove(m_vsRemovedKinks);
  }
}

//----------------------------------------------------------------------------------------
//
void CCommandRemoveFetishes::redo()
{
  if (!m_pGuard.isNull())
  {
    m_fnAdd(m_vsRemovedKinks);
  }
}
//----------------------------------------------------------------------------------------
//
int CCommandRemoveFetishes::id() const
{
  return EEditorCommandId::eRemoveFetishes;
}

//----------------------------------------------------------------------------------------
//
bool CCommandRemoveFetishes::mergeWith(const QUndoCommand* pOther)
{
  if (nullptr == pOther || id() != pOther->id())
  {
    return false;
  }

  const CCommandRemoveFetishes* pOtherCasted = dynamic_cast<const CCommandRemoveFetishes*>(pOther);
  if (nullptr == pOtherCasted) { return false; }

  m_vsRemovedKinks << pOtherCasted->m_vsRemovedKinks;
  setText(QString("Removed %1 kinks").arg(m_vsRemovedKinks.size()));
  return true;
}
