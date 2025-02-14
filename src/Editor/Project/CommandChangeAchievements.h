#ifndef CCOMMANDCHANGEACHIEVEMENTS_H
#define CCOMMANDCHANGEACHIEVEMENTS_H

#include "Systems/DatabaseInterface/SaveData.h"

#include <QUndoCommand>

#include <functional>
#include <memory>

class CDatabaseManager;

using tfnOnAddAchievement = std::function<void(const SSaveDataData&)>;
using tfnOnChangeAchievement = std::function<void(const SSaveDataData&,const SSaveDataData&)>;
using tfnOnRemoveAchievement = std::function<void(const SSaveDataData&)>;

//----------------------------------------------------------------------------------------
//
class CCommandAddAchievements : public QUndoCommand
{
public:
  CCommandAddAchievements(const SSaveDataData& newData, const QString& sCurrentProject,
                          const tfnOnAddAchievement& fnAdd, const tfnOnRemoveAchievement& fnRemove);
  ~CCommandAddAchievements();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  tfnOnAddAchievement             m_fnAdd;
  tfnOnRemoveAchievement          m_fnRemove;

  std::weak_ptr<CDatabaseManager> m_wpDbManager;
  QString                         m_sCurrentProject;
  SSaveDataData                   m_newData;
};

//----------------------------------------------------------------------------------------
//
class CCommandChangeAchievements : public QUndoCommand
{
public:
  CCommandChangeAchievements(const SSaveDataData& newData, const SSaveDataData& oldData,
                             const QString& sCurrentProject, const tfnOnChangeAchievement& fnChange);
  ~CCommandChangeAchievements();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  void ChangeAchievement(SSaveDataData oldData, SSaveDataData newData);

  tfnOnChangeAchievement          m_fnChange;

  std::weak_ptr<CDatabaseManager> m_wpDbManager;
  QString                         m_sCurrentProject;
  SSaveDataData                   m_oldData;
  SSaveDataData                   m_newData;
};

//----------------------------------------------------------------------------------------
//
class CCommandRemoveAchievements : public QUndoCommand
{
public:
  CCommandRemoveAchievements(const SSaveDataData& oldData, const QString& sCurrentProject,
                             const tfnOnAddAchievement& fnAdd, const tfnOnRemoveAchievement& fnRemove);
  ~CCommandRemoveAchievements();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  tfnOnAddAchievement             m_fnAdd;
  tfnOnRemoveAchievement          m_fnRemove;

  std::weak_ptr<CDatabaseManager> m_wpDbManager;
  QString                         m_sCurrentProject;
  SSaveDataData                   m_oldData;
};

#endif // CCOMMANDCHANGEACHIEVEMENTS_H
