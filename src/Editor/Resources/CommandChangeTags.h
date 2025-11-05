#ifndef CCOMMANDCHANGETAGS_H
#define CCOMMANDCHANGETAGS_H

#include "Systems/Database/Project.h"

#include <QStringList>
#include <QUndoCommand>
#include <functional>

class CDatabaseManager;

class CCommandAddTag : public QUndoCommand
{
public:
  CCommandAddTag(const QString& sProject, const QString& sResource,
                 const tspTag& spTag);
  ~CCommandAddTag();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  QString                         m_sCurrentProject;
  std::weak_ptr<CDatabaseManager> m_wpDbManager;
  QString                         m_sResource;
  tspTag                          m_spTag;
};

//----------------------------------------------------------------------------------------
//
class CCommandRemoveTag : public QUndoCommand
{
public:
  CCommandRemoveTag(const QString& sProject, const QString& sResource,
                     const tspTag& spTag);
  ~CCommandRemoveTag();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  QString                         m_sCurrentProject;
  std::weak_ptr<CDatabaseManager> m_wpDbManager;
  QString                         m_sResource;
  tspTag                          m_spTag;
};

#endif // CCOMMANDCHANGETAGS_H
