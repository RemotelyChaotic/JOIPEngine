#include "CommandChangeAchievements.h"
#include "Application.h"

#include "Editor/EditorCommandIds.h"

#include "Systems/DatabaseManager.h"
#include "Systems/Project.h"
#include "Systems/SaveData.h"

namespace
{
  void AddAchievement(std::shared_ptr<CDatabaseManager> spDbManager,
                      const tfnOnAddAchievement& fnAdd,
                      const SSaveDataData& newData,
                      const QString& sCurrentProject)
  {
    if (nullptr != spDbManager)
    {
      auto spProj = spDbManager->FindProject(sCurrentProject);
      if (nullptr != spProj)
      {
        const QString sAch =
            spDbManager->AddAchievement(spProj, newData.m_sName, newData.m_sDescribtion,
                                        newData.m_type._to_integral(), newData.m_sResource,
                                        newData.m_data);
        if (!sAch.isEmpty() && nullptr != fnAdd)
        {
          fnAdd(newData);
        }
      }
    }
  }

  //--------------------------------------------------------------------------------------
  //
  void RemoveAchievement(std::shared_ptr<CDatabaseManager> spDbManager,
                      const tfnOnRemoveAchievement& fnRemove,
                      const SSaveDataData& oldData,
                      const QString& sCurrentProject)
  {
    if (nullptr != spDbManager)
    {
      auto spProj = spDbManager->FindProject(sCurrentProject);
      if (nullptr != spProj)
      {
        spDbManager->RemoveAchievement(spProj, oldData.m_sName);
        if (nullptr != fnRemove)
        {
          fnRemove(oldData);
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
CCommandAddAchievements::CCommandAddAchievements(const SSaveDataData& newData,
                                                 const QString& sCurrentProject,
                                                 const tfnOnAddAchievement& fnAdd,
                                                 const tfnOnRemoveAchievement& fnRemove) :
  QUndoCommand(QObject::tr("Added achievement: %1").arg(newData.m_sName)),
  m_fnAdd(fnAdd),
  m_fnRemove(fnRemove),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_sCurrentProject(sCurrentProject),
  m_newData(newData)
{
}
CCommandAddAchievements::~CCommandAddAchievements() = default;

//----------------------------------------------------------------------------------------
//
void CCommandAddAchievements::undo()
{
  RemoveAchievement(m_wpDbManager.lock(), m_fnRemove, m_newData, m_sCurrentProject);
}

//----------------------------------------------------------------------------------------
//
void CCommandAddAchievements::redo()
{
  AddAchievement(m_wpDbManager.lock(), m_fnAdd, m_newData, m_sCurrentProject);
}

//----------------------------------------------------------------------------------------
//
int CCommandAddAchievements::id() const
{
  return EEditorCommandId::eAddAchievement;
}

//----------------------------------------------------------------------------------------
//
bool CCommandAddAchievements::mergeWith(const QUndoCommand*)
{
  return false;
}

CCommandChangeAchievements::CCommandChangeAchievements(const SSaveDataData& newData,
                                                       const SSaveDataData& oldData,
                                                       const QString& sCurrentProject,
                                                       const tfnOnChangeAchievement& fnChange) :
  QUndoCommand(QObject::tr("Changed achievement: %1").arg(oldData.m_sName)),
  m_fnChange(fnChange),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_sCurrentProject(sCurrentProject),
  m_oldData(oldData),
  m_newData(newData)
{
  if (newData.m_sName != oldData.m_sName)
  {
    setText(QObject::tr("Changed and renamed achievement: %1 to: %2")
                     .arg(oldData.m_sName).arg(newData.m_sName));
  }
}
CCommandChangeAchievements::~CCommandChangeAchievements() = default;

//----------------------------------------------------------------------------------------
//
void CCommandChangeAchievements::undo()
{
  ChangeAchievement(m_newData, m_oldData);
}

//----------------------------------------------------------------------------------------
//
void CCommandChangeAchievements::redo()
{
  ChangeAchievement(m_oldData, m_newData);
}

//----------------------------------------------------------------------------------------
//
int CCommandChangeAchievements::id() const
{
  return EEditorCommandId::eChangeAchievement;
}

//----------------------------------------------------------------------------------------
//
bool CCommandChangeAchievements::mergeWith(const QUndoCommand* pOther)
{
  if (nullptr == pOther || id() != pOther->id())
  {
    return false;
  }

  const CCommandChangeAchievements* pOtherCasted =
      dynamic_cast<const CCommandChangeAchievements*>(pOther);
  if (nullptr == pOtherCasted) { return false; }
  m_newData = pOtherCasted->m_newData;
  if (m_newData.m_sName != m_oldData.m_sName)
  {
    setText(QObject::tr("Changed and renamed achievement: %1 to: %2")
                .arg(m_oldData.m_sName).arg(m_newData.m_sName));
  }
  return true;
}

//----------------------------------------------------------------------------------------
//
void CCommandChangeAchievements::ChangeAchievement(SSaveDataData oldData, SSaveDataData newData)
{
  if (auto spDbManager = m_wpDbManager.lock())
  {
    auto spProj = spDbManager->FindProject(m_sCurrentProject);
    if (nullptr != spProj)
    {
      qint32 iId = -1;
      {
        QReadLocker lockerProj(&spProj->m_rwLock);
        iId = spProj->m_iId;
      }
      auto spOldAchData = spDbManager->FindAchievementInProject(spProj, oldData.m_sName);
      if (nullptr != spOldAchData)
      {
        QWriteLocker l(&spOldAchData->m_rwLock);
        bool bNameChanged = oldData.m_sName != newData.m_sName;
        *dynamic_cast<SSaveDataData*>(spOldAchData.get()) = newData;
        l.unlock();
        if (bNameChanged)
        {
          spDbManager->RenameAchievement(spProj, oldData.m_sName, newData.m_sName);
        }
        emit spDbManager->SignalAchievementDataChanged(iId, newData.m_sName);
        if (nullptr != m_fnChange)
        {
          m_fnChange(oldData, newData);
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
CCommandRemoveAchievements::CCommandRemoveAchievements(const SSaveDataData& oldData,
                                                       const QString& sCurrentProject,
                                                       const tfnOnAddAchievement& fnAdd,
                                                       const tfnOnRemoveAchievement& fnRemove) :
  QUndoCommand(QObject::tr("Removed achievement: %1").arg(oldData.m_sName)),
  m_fnAdd(fnAdd),
  m_fnRemove(fnRemove),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_sCurrentProject(sCurrentProject),
  m_oldData(oldData)
{
}
CCommandRemoveAchievements::~CCommandRemoveAchievements() = default;

//----------------------------------------------------------------------------------------
//
void CCommandRemoveAchievements::undo()
{
  AddAchievement(m_wpDbManager.lock(), m_fnAdd, m_oldData, m_sCurrentProject);
}

//----------------------------------------------------------------------------------------
//
void CCommandRemoveAchievements::redo()
{
  RemoveAchievement(m_wpDbManager.lock(), m_fnRemove, m_oldData, m_sCurrentProject);
}

//----------------------------------------------------------------------------------------
//
int CCommandRemoveAchievements::id() const
{
  return EEditorCommandId::eRemoveAchievement;
}

//----------------------------------------------------------------------------------------
//
bool CCommandRemoveAchievements::mergeWith(const QUndoCommand*)
{
  return false;
}
